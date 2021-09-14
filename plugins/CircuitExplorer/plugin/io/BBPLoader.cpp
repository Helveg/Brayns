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
#include <plugin/io/bbploader/simulation/CompartmentSimulation.h>
#include <plugin/io/bbploader/simulation/SpikeSimulation.h>
#include <plugin/io/util/ProgressReport.h>
#include <plugin/io/util/TransferFunctionUtils.h>

#include <brain/brain.h>
#include <brion/brion.h>

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

    const auto expectedSize = static_cast<size_t>(allGids.size() * lc.percentage);
    const auto skipFactor = static_cast<size_t>(static_cast<float>(allGids.size())
                                                / static_cast<float>(expectedSize));
    brain::GIDSet finalList;
    auto it = allGids.begin();
    while(it != allGids.end())
    {
        finalList.insert(*it);
        std::advance(it, skipFactor);
    }

    return finalList;
}

inline brayns::ModelDescriptorPtr __loadSynapse(const std::string& path,
                                                const brain::Circuit& circuit,
                                                const brain::GIDSet& gids,
                                                const bool afferent,
                                                const std::vector<MorphologyInstancePtr>& cells,
                                                ProgressReport& pr,
                                                brayns::ModelPtr&& model)
{
    const std::string msg = afferent? "Loading afferent synapses" : "Loading efferent synapses";
    auto aslpr = pr.nextSubProgress(msg, gids.size() * 2);
    const auto synapses = SynapseLoader::load(circuit, gids, afferent, aslpr);
    for(size_t i = 0; i < cells.size(); ++i)
    {
        synapses[i]->mapToCell(*cells[i]);
        synapses[i]->addToModel(*model);
        aslpr.tick();
    }
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
    return modelDescriptor;
}
}

BBPLoader::BBPLoader(brayns::Scene& scene, CircuitColorManager& colorManager)
 : brayns::Loader(scene)
 , _colorManager(colorManager)
{
    PLUGIN_INFO << "Registering loader :" << getName() << std::endl;
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
BBPLoader::importFromBlob(brayns::Blob&& blob,
                          const brayns::LoaderProgress& callback,
                          const brayns::PropertyMap& properties) const
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

    // CREATE MODEL DESCRIPTOR
    // -------------------------------------------------------------------------------------------
    auto amgpr = pr.nextSubProgress("Generating geometry", gids.size());
    for(size_t i = 0; i < cells.size(); ++i)
        cells[i]->addToModel(*cellModel);
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

    // LOAD AFFERENT (IF REQUESTED)
    // -------------------------------------------------------------------------------------------
    if(loadConfig.loadAfferent)
    {
        auto model = __loadSynapse(path, circuit, gids, true, cells, pr, _scene.createModel());
        result.push_back(model);
    }

    // LOAD EFFERENT (IF REQUESTED)
    // -------------------------------------------------------------------------------------------
    if(loadConfig.loadEfferent)
    {
        auto model = __loadSynapse(path, circuit, gids, false, cells, pr, _scene.createModel());
        result.push_back(model);
    }

    return result;
}
