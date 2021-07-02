/* Copyright (c) 2015-2021, EPFL/Blue Brain Project
 * All rights reserved. Do not distribute without permission.
 * Responsible Author: Nadir Roman <nadir.romanguerrero@epfl.ch>
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

#include "SonataLoader.h"

#include "SonataLoaderProperties.h"
#include "data/SonataPopulation.h"
#include "data/SonataSimulationMapping.h"

#include "../morphology/Morphology.h"
#include "../morphology/MorphologyPipeline.h"
#include "../morphology/builders/SDFGeometryBuilder.h"
#include "../morphology/pipeline/RadiusMultiplier.h"
#include "../morphology/pipeline/RadiusSmoother.h"

#include "../../CircuitExplorerPlugin.h"
#include "../../../common/log.h"

#include <brayns/common/utils/stringUtils.h>

namespace sonata
{

namespace
{
struct PopulationModel
{
    brayns::ModelPtr model;
    brayns::Vector3f modelWorldPosition;
    std::string reportName;
    std::string reportType;
    std::string nodeSetsString;
    uint64_t numCells;
};

auto createModelDescriptor(const std::string& path,
                            PopulationModel& popModel)
{
    brayns::ModelMetadata metadata = {
        {"Report", popModel.reportName},
        {"Report type", popModel.reportType},
        {"NodeSets", popModel.nodeSetsString},
        {"Number of neurons", std::to_string(popModel.numCells)},
        {"CircuitPath", path}};

    brayns::ModelDescriptorPtr modelDescriptor;
    brayns::Transformation transformation;
    transformation.setRotationCenter(popModel.modelWorldPosition);
    modelDescriptor =
        std::make_shared<brayns::ModelDescriptor>(std::move(popModel.model), "Circuit",
                                                  path,
                                                  metadata);
    modelDescriptor->setTransformation(transformation);

    /*
    // Clean the circuit mapper associated with this model
    modelDescriptor->onRemoved([plptr = _pluginPtr](const brayns::ModelDescriptor& remMod)
    {
        plptr->releaseCircuitMapper(remMod.getModelID());
    });

    objMapper.setSourceModel(modelDescriptor);
    objMapper.onCircuitColorFinish(colorScheme, morphologyScheme);

    this->_pluginPtr->addCircuitMapper(std::move(objMapper));
    */
    return modelDescriptor;
} 

auto createMorphologyPipeline(const PopulationLoadConfig& populationProps)
{
    morphology::MorphologyPipeline pipeline;

    if(populationProps.morphologyRadius != 1.f)
        pipeline.registerStage<morphology::RadiusMultiplier>(populationProps.morphologyRadius);

    if(populationProps.morphologyMode == "vanilla")
    {

    }
    else if(populationProps.morphologyMode == "smooth")
    {
        pipeline.setGeometryBuilderClass<morphology::SDFGeometryBuilder>();
        pipeline.registerStage<morphology::RadiusSmoother>();
    }
    else if(populationProps.morphologyMode == "samples")
    {

    }

    return pipeline;
}

auto loadPopulation(const bbp::sonata::CircuitConfig& config,
                    const PopulationLoadConfig& popProps,
                    const brayns::LoaderProgress& cb,
                    const double startLoadProgress,
                    const double endLoadProgress,
                    brayns::Scene& scene)
{
    // Extra setting stored on configuration file (morphology_dir)
    const auto populationSettings = config.getNodePopulationProperties(popProps.name);

    if(populationSettings.type != "biophysical")
        throw std::runtime_error("Only biophysical population types are supported at the momment");

    // Loaded data from input settings
    const data::SonataPopulation population (config, popProps);

    // Simulation, if any
    const auto simulationMapping = data::SonataSimulationMapping::compute(population,
                                                                          popProps.simulationPath,
                                                                          popProps.simulationType);

    // Maps morphology class to all cell indices that uses that class
    std::unordered_map<std::string, std::vector<uint64_t>> morphologyMap;
    for(size_t i = 0; i < population.getNumLoadedCells(); ++i)
    {
        const auto& cell = population.getCell(i);
        morphologyMap[cell.morphologyClass].push_back(cell.index);
    }

    const double progressChunk = (endLoadProgress - startLoadProgress) /
            static_cast<double>(morphologyMap.size());
    double currentProgress = startLoadProgress;
    const std::string loadingPlaceholder = "Loading " + population.name() + " morphologies";

    auto model = scene.createModel();
    brayns::Boxf fastBounds;
    fastBounds.reset();

    const morphology::MorphologyPipeline pipeline = createMorphologyPipeline(popProps);

    for(const auto& entry : morphologyMap)
    {
        cb.updateProgress(loadingPlaceholder, static_cast<float>(currentProgress));

        // Load morphology
        const auto morphPath = populationSettings.morphologiesDir + "/" + entry.first + ".swc";
        const auto builder = pipeline.importMorphology(morphPath,
                                                       popProps.morphologySections);

        // Instantiate the morphology for every cell with such morphology class
        for(const auto cellIndex : entry.second)
        {
            // Build cell morphology geometry
            const auto& cell = population.getCell(cellIndex);
            auto morphologyInstance = builder->instantiate(cell.translation, cell.rotation);

            // Add synapses, if any
            for(const auto& afferentPop : popProps.afferentPopulations)
            {
                const auto& afferentSynapses = population.getAfferentSynapses(cell, afferentPop);
                for(const auto& synapse : afferentSynapses)
                    morphologyInstance->addSynapse(afferentPop,
                                                   synapse.surfacePos,
                                                   synapse.edgeId,
                                                   synapse.sectionId,
                                                   true);
            }
            for(const auto& efferentPop : popProps.efferentPopulations)
            {
                const auto& efferentSynapses = population.getEfferentSynapses(cell, efferentPop);
                for(const auto& synapse : efferentSynapses)
                    morphologyInstance->addSynapse(efferentPop,
                                                   synapse.surfacePos,
                                                   synapse.edgeId,
                                                   synapse.sectionId,
                                                   false);
            }

            // Map simulation, if any
            if(!simulationMapping.empty())
            {
                const auto& cellMapping = simulationMapping[cellIndex];
                morphologyInstance->mapSimulation(cellMapping.globalOffset,
                                                  cellMapping.sectionsOffsets,
                                                  cellMapping.sectionsCompartments);
            }

            // Add to the model
            morphologyInstance->addToModel(*model);

            // Update model bounds
            fastBounds.merge(brayns::Vector3f(cell.translation));
        }

        currentProgress += progressChunk;
    }

    PopulationModel pm;
    pm.model = std::move(model);
    pm.modelWorldPosition = fastBounds.getCenter();
    pm.nodeSetsString = brayns::string_utils::join(popProps.nodeSets, ",");
    pm.numCells = population.getNumLoadedCells();
    return pm;
}

} // namespace

SonataLoader::SonataLoader(brayns::Scene& scene, CircuitExplorerPlugin* pluginPtr)
 : brayns::Loader(scene)
 , _pluginPtr(pluginPtr)
{
    PLUGIN_INFO << "Registering " << getName() << std::endl;
}

std::vector<std::string> SonataLoader::getSupportedExtensions() const
{
    return std::vector<std::string> {".json"};
}

bool SonataLoader::isSupported(const std::string&, const std::string& extension) const
{
    std::string extCopy = extension;
    std::transform(extCopy.begin(), extCopy.end(), extCopy.begin(), [](const char c)
    {
        return std::tolower(c);
    });
    return extCopy == "json";
}

std::string SonataLoader::getName() const
{
    return std::string ("Sonata circuit loader");
}

brayns::PropertyMap SonataLoader::getProperties() const
{
    return SonataLoaderProperties::getPropertyList();
}

std::vector<brayns::ModelDescriptorPtr>
SonataLoader::importFromBlob(brayns::Blob&& blob,
                             const brayns::LoaderProgress& callback,
                             const brayns::PropertyMap& properties) const
{
    throw std::runtime_error("Sonata loader: import from blob not supported");
}

std::vector<brayns::ModelDescriptorPtr>
SonataLoader::importFromFile(const std::string& path,
                             const brayns::LoaderProgress& callback,
                             const brayns::PropertyMap& props) const
{
    PLUGIN_INFO << "Sonata multi-population loader: Loading " << path << std::endl;
        const bbp::sonata::CircuitConfig config = bbp::sonata::CircuitConfig::fromFile(path);

    // Check input loading paraemters <-> disk files sanity
    callback.updateProgress("Parsing input parameters", 0.f);
    const SonataLoaderProperties loaderProps (config, props);

    const auto numPopulations = loaderProps.nodePopulations().size();

    // Load each population
    const double progressChunk = 1.0 / static_cast<double>(numPopulations);
    double currentProgress = 0.f;

    std::vector<brayns::ModelDescriptorPtr> result;
    result.reserve(numPopulations);

    for(const auto& nodePopConfig : loaderProps.nodePopulations())
    {
        callback.updateProgress("Loading population " + nodePopConfig.name, currentProgress);
        auto model = _scene.createModel();

        // Load the morphology, synapses and simulation into a model
        const float startProgress = currentProgress;
        const float endProgress = startProgress + progressChunk;
        auto pm = loadPopulation(config,
                                  nodePopConfig,
                                  callback,
                                  currentProgress,
                                  endProgress,
                                  _scene);
        // Create the model descriptor and add it to the scene
        result.push_back(createModelDescriptor(path, pm));

        currentProgress += progressChunk;
    }

    return result;

    return {};
}

} // namespace sonata
