/* Copyright (c) 2018-2019, EPFL/Blue Brain Project
 * All rights reserved. Do not distribute without permission.
 * Responsible Author: Cyrille Favreau <cyrille.favreau@epfl.ch>
 *
 * This file is part of the circuit explorer for Brayns
 * <https://github.com/favreau/Brayns-UC-CircuitExplorer>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "BBPLoader.h"

#include <brayns/common/Timer.h>
#include <brayns/common/utils/filesystem.h>
#include <brayns/engine/Scene.h>

#include <plugin/api/Log.h>
#include <plugin/api/MaterialUtils.h>
#include <plugin/io/bbploader/BBPLoaderProperties.h>
#include <plugin/io/bbploader/CellLoader.h>
#include <plugin/io/bbploader/SynapseLoader.h>
#include <plugin/io/bbploader/colorhandlers/NeuronColorHandler.h>
#include <plugin/io/bbploader/colorhandlers/SynapseColorHandler.h>
#include <plugin/io/bbploader/simulation/CompartmentSimulation.h>
#include <plugin/io/bbploader/simulation/SpikeSimulation.h>
#include <plugin/io/util/ProgressReport.h>
#include <plugin/io/util/TransferFunctionUtils.h>

#include <brain/brain.h>
#include <brion/brion.h>

#include <iostream>
#include <fstream>
#include <unistd.h>

using namespace bbploader;

namespace
{
inline std::unique_ptr<Simulation> __intantiateSimulation(const brion::BlueConfig& config,
                                                          const BBPCircuitLoadConfig& lc,
                                                          const brain::GIDSet& inputGids) noexcept
{
    switch(lc.reportType)
    {
    case SimulationType::COMPARTMENT:
        return std::make_unique<CompartmentSimulation>(
                    config.getReportSource(lc.reportName).getPath(), inputGids);
    case SimulationType::SPIKES:
        return std::make_unique<SpikeSimulation>(config.getSpikeSource().getPath(),
                                                 inputGids,
                                                 lc.spikeTransitionTime);
    default:
        return {nullptr};
    }
}

inline brain::GIDSet __computeInitialGIDs(const brion::BlueConfig& config,
                                          const brain::Circuit& circuit,
                                          const BBPCircuitLoadConfig& lc)
{
    if(!lc.gids.empty())
        return brain::GIDSet(lc.gids.begin(), lc.gids.end());

    brain::GIDSet result;
    std::vector<std::string> targets;
    if(!lc.targets.empty())
        targets = lc.targets;
    else
        targets = {config.getCircuitTarget()};

    brain::GIDSet allGids;
    for(const auto& target : targets)
    {
        const auto tempGids = circuit.getGIDs(target);
        allGids.insert(tempGids.begin(), tempGids.end());
    }

    if(lc.percentage >= 1.f)
        return allGids;

    const auto expectedSize = static_cast<size_t>(allGids.size() * lc.percentage);
    const auto skipFactor = static_cast<size_t>(static_cast<float>(allGids.size())
                                                / static_cast<float>(expectedSize));
    brain::GIDSet finalList;
    auto it = finalList.begin();
    auto allIt = allGids.begin();
    while(allIt != allGids.end())
    {
        finalList.insert(it, *allIt);
        ++it;
        size_t counter {0};
        while(counter++ < skipFactor && allIt != allGids.end())
            ++allIt;
    }

    return finalList;
}

inline brayns::ModelDescriptorPtr __loadSynapse(const std::string& path,
                                                const brain::Circuit& circuit,
                                                const brain::GIDSet& gids,
                                                const bool afferent,
                                                const std::vector<MorphologyInstancePtr>& cells,
                                                ProgressReport& pr,
                                                brayns::ModelPtr&& model,
                                                CircuitColorManager& colorManager)
{
    const std::string msg = afferent? "Loading afferent synapses" : "Loading efferent synapses";
    auto aslpr = pr.nextSubProgress(msg, gids.size() * 2);
    auto synapses = SynapseLoader::load(circuit, gids, afferent, aslpr);
    std::vector<ElementMaterialMap::Ptr> synapseMatMap (gids.size());
    for(size_t i = 0; i < cells.size(); ++i)
    {
        synapses[i]->mapToCell(*cells[i]);
        synapseMatMap[i] = synapses[i]->addToModel(*model);
        synapses[i].reset(nullptr);
        aslpr.tick();
    }
    if(model->empty())
        return {nullptr};

    model->updateBounds();

    brayns::ModelMetadata metadata =
    {
        {"Synapse type", afferent? "Afferent" : "Efferent"},
        {"CircuitPath", path}
    };

    brayns::Transformation transformation;
    transformation.setRotationCenter(model->getBounds().getCenter());
    brayns::ModelDescriptorPtr modelDescriptor =
        std::make_shared<brayns::ModelDescriptor>(std::move(model), "Synapses", path, metadata);
    modelDescriptor->setTransformation(transformation);
    modelDescriptor->setName(afferent? "Afferent synapses" : "Efferent synapses");

    auto synapseColorHandler = std::make_unique<SynapseColorHandler>(modelDescriptor.get());
    synapseColorHandler->setElements(std::vector<uint64_t>(gids.begin(), gids.end()),
                                     std::move(synapseMatMap));
    modelDescriptor->onRemoved([cmptr = &colorManager](const brayns::ModelDescriptor& m)
    {
        cmptr->unregisterHandler(m.getModelID());
    });
    colorManager.registerHandler(std::move(synapseColorHandler));
    return modelDescriptor;
}

inline std::string __getCircuitFilePath(const brion::BlueConfig& config)
{
    const auto csrc = config.getCircuitSource().getPath();
    if(fs::exists(csrc))
        return csrc;
    const auto ssrc = config.getCellLibrarySource().getPath();
    if(fs::exists(ssrc))
        return ssrc;

    return "";
}
}

BBPLoader::BBPLoader(brayns::Scene& scene, CircuitColorManager& colorManager)
 : brayns::Loader(scene)
 , _colorManager(colorManager)
{
    PLUGIN_INFO << "Registering loader: " << getName() << std::endl;
}

std::vector<std::string> BBPLoader::getSupportedExtensions() const
{
    static const strings LOADER_EXTENSIONS =
    {
        "BlueConfig",
        "BlueConfig3",
        "CircuitConfig",
        "CircuitConfig_nrn"
    };
    return LOADER_EXTENSIONS;
}

bool BBPLoader::isSupported(const std::string& filename, const std::string& extension) const
{
    const auto containsKeyword = [](const std::string& matcher)
    {
        const auto lcm = brayns::string_utils::toLowercase(matcher);
        if(lcm.find("blueconfig") != std::string::npos
                || lcm.find("circuitconfig") != std::string::npos)
            return true;

        return false;
    };

    return containsKeyword(fs::path(filename).filename()) || containsKeyword(extension);
}

std::string BBPLoader::getName() const
{
    return "BBP loader";
}

brayns::PropertyMap BBPLoader::getProperties() const
{
    return BBPLoaderProperties::getPropertyList();
}

std::vector<brayns::ModelDescriptorPtr>
BBPLoader::importFromBlob(brayns::Blob&&,
                          const brayns::LoaderProgress&,
                          const brayns::PropertyMap&) const
{
    throw std::runtime_error("BBP loader: import from blob not supported");
}

std::vector<brayns::ModelDescriptorPtr>
BBPLoader::importFromFile(const std::string& path,
                          const brayns::LoaderProgress& callback,
                          const brayns::PropertyMap& properties) const
{
    brayns::Timer timer;
    PLUGIN_INFO << getName() << ": Loading " << path << std::endl;

    const brion::BlueConfig config (path);
    const auto result = importFromBlueConfig(path, callback, properties, config);

    PLUGIN_INFO << getName() << ": Done in " << timer.elapsed() << " second(s)" << std::endl;
    return result;
}

std::vector<brayns::ModelDescriptorPtr>
BBPLoader::importFromBlueConfig(const std::string& path,
                                const brayns::LoaderProgress& callback,
                                const brayns::PropertyMap& properties,
                                const brion::BlueConfig& config) const
{
    std::vector<brayns::ModelDescriptorPtr> result;

    // INITIALIZE DATA ACCESSORS
    // -------------------------------------------------------------------------------------------
    const brain::Circuit circuit (config);
    const auto loadConfig = BBPLoaderProperties::checkAndParse(config, properties);

    brayns::ModelPtr cellModel = _scene.createModel();

    // Configure progress reporter
    size_t loadChunks = 3;
    loadChunks += loadConfig.reportType != SimulationType::NONE? 1 : 0;
    loadChunks += loadConfig.loadAfferent? 2 : 0;
    loadChunks += loadConfig.loadEfferent? 2 : 0;
    ProgressReport pr (callback, 0.f, 1.f, loadChunks);

    // COMPUTE INITIAL GIDS
    // -------------------------------------------------------------------------------------------
    pr.nextSubProgress("Processing GIDs to load");
    auto gids = __computeInitialGIDs(config, circuit, loadConfig);

    // Load simulation, intersect initial gids with simulation gids
    const auto simulation = __intantiateSimulation(config, loadConfig, gids);
    if(simulation)
        gids = simulation->getReportGids();

    if(gids.empty())
        throw std::runtime_error("BBPLoader: No GIDs selected. Empty circuits not supported");

    // LOAD CELLS
    // -------------------------------------------------------------------------------------------
    auto lcpr = pr.nextSubProgress("Loading cells", gids.size());
    auto cells = CellLoader::load(loadConfig, gids, circuit, lcpr);

    // MAP SIMULATION (IF ANY)
    // -------------------------------------------------------------------------------------------
    if(simulation)
    {
        auto lspr = pr.nextSubProgress("Loading simulation", gids.size());
        const auto mapping = simulation->getMapping(gids);
        for(size_t i = 0; i < mapping.size(); ++i)
        {
            const auto& cm = mapping[i];
            cells[i]->mapSimulation(cm.globalOffset, cm.offsets, cm.compartments);
            lspr.tick();
        }
        cellModel->setSimulationHandler(simulation->createHandler());
        TransferFunctionUtils::set(_scene.getTransferFunction());
    }

    // LOAD AFFERENT (IF REQUESTED)
    // -------------------------------------------------------------------------------------------
    if(loadConfig.loadAfferent)
    {
        auto model = __loadSynapse(path,
                                   circuit,
                                   gids,
                                   true,
                                   cells,
                                   pr,
                                   _scene.createModel(),
                                   _colorManager);
        if(model)
            result.push_back(model);
    }

    // LOAD EFFERENT (IF REQUESTED)
    // -------------------------------------------------------------------------------------------
    if(loadConfig.loadEfferent)
    {
        auto model = __loadSynapse(path,
                                   circuit,
                                   gids,
                                   false,
                                   cells,
                                   pr,
                                   _scene.createModel(),
                                   _colorManager);
        if(model)
            result.push_back(model);
    }

    // CREATE MODEL DESCRIPTOR
    // -------------------------------------------------------------------------------------------
    auto amgpr = pr.nextSubProgress("Generating geometry", gids.size());
    std::vector<ElementMaterialMap::Ptr> cellMatMap (gids.size());
    for(size_t i = 0; i < cells.size(); ++i)
    {
        cellMatMap[i] = cells[i]->addToModel(*cellModel);
        cells[i].reset(nullptr);
        amgpr.tick();
    }
    cells.clear();
    cells.shrink_to_fit();

    cellModel->updateBounds();
    if(simulation)
        CircuitExplorerMaterial::setSimulationColorEnabled(*cellModel, true);

    brayns::ModelMetadata metadata =
    {
        {"Report", loadConfig.reportName},
        {"Report type", EnumWrapper<SimulationType>().toString(loadConfig.reportType)},
        {"Targets", properties.getProperty<std::string>(PROP_TARGETS.name)},
        {"GIDs", properties.getProperty<std::string>(PROP_GIDS.name)},
        {"Number of neurons", std::to_string(gids.size())},
        {"Percentage", std::to_string(loadConfig.percentage)},
        {"CircuitPath", path}
    };

    brayns::Transformation transformation;
    transformation.setRotationCenter(cellModel->getBounds().getCenter());
    brayns::ModelDescriptorPtr modelDescriptor =
        std::make_shared<brayns::ModelDescriptor>(std::move(cellModel), "Circuit", path, metadata);
    modelDescriptor->setTransformation(transformation);
    result.push_back(modelDescriptor);

    // CREATE COLOR HANDLER
    // -------------------------------------------------------------------------------------------
    auto cellColorHandler = std::make_unique<NeuronColorHandler>(modelDescriptor.get(),
                                                                 __getCircuitFilePath(config),
                                                                 config.getCircuitPopulation());
    cellColorHandler->setElements(std::vector<uint64_t>(gids.begin(), gids.end()),
                                  std::move(cellMatMap));
    modelDescriptor->onRemoved([cmptr = &_colorManager](const brayns::ModelDescriptor& m)
    {
        cmptr->unregisterHandler(m.getModelID());
    });
    _colorManager.registerHandler(std::move(cellColorHandler));

    return result;
}
