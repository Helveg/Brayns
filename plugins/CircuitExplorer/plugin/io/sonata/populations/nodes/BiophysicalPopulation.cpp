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

#include "BiophysicalPopulation.h"

#include "../../data/SonataCells.h"
#include "../../data/SonataSelection.h"
#include "../../data/SonataSimulationMapping.h"
#include "../../data/SonataSynapses.h"

#include "../../../morphology/MorphologyPipeline.h"
#include "../../../morphology/builders/SDFGeometryBuilder.h"
#include "../../../morphology/pipeline/RadiusMultiplier.h"
#include "../../../morphology/pipeline/RadiusSmoother.h"

namespace sonata
{
namespace population
{
namespace
{
bbp::sonata::Selection
selectCells(const bbp::sonata::CircuitConfig& config,
            const PopulationLoadConfig& properties)
{
    data::NodeSelection selection;
    selection.select(config, properties.name, properties.nodeSets);
    selection.select(properties.nodeIds);
    selection.select(properties.simulationType, properties.simulationPath, properties.name);
    return selection.intersection(properties.percentage);
}

std::unique_ptr<data::SonataSimulationMapping>
createSimulationMapper(const PopulationLoadConfig& popProps)
{

}

struct SynapsePopulation
{
    std::vector<brayns::Vector3f> positions;
    std::vector<uint64_t> srcNodes;
    std::vector<uint64_t> targetNodes;
    std::vector<uint64_t> synapseId;
    std::vector<int32_t> sectionId;
};

auto readSynapses(const bbp::sonata::CircuitConfig& config,
                  const std::vector<std::string>& populations,
                  const float percentage,
                  const bbp::sonata::Selection& nodeSelection)
{
    std::unordered_map<std::string, SynapsePopulation> result;

    for(const std::string& population : populations)
    {
        auto& populationData = result[population];
        const auto& edges = config.getEdgePopulation(population);

    }

    return result;
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
}

void BiophysicalPopualtion::importPopulation(const bbp::sonata::CircuitConfig& config,
                                             const PopulationLoadConfig& popProps,
                                             brayns::Model& dst,
                                             brayns::LoaderProgress& updateCb) const
{
    const auto nodeSelection = selectCells(config, popProps);
    const auto nodes = config.getNodePopulation(popProps.name);

    // Extra setting stored on configuration file (morphology_dir)
    const auto populationSettings = config.getNodePopulationProperties(popProps.name);

    // Simulation, if any
    std::vector<data::SimulationMapping> mapping;
    const auto simulationMapping = createSimulationMapper(popProps);
    if(simulationMapping)
        mapping = simulationMapping->compute();

    const auto nodeIds = nodeSelection.flatten();
    const auto morphologies = data::SonataCells::getMorphologies(nodes, nodeSelection);
    const auto positions = data::SonataCells::getPositions(nodes, nodeSelection);
    const auto rotations = data::SonataCells::getRotations(nodes, nodeSelection);

    // Maps morphology class to all cell indices that uses that class
    std::unordered_map<std::string, std::vector<size_t>> morphologyMap;
    for(size_t i = 0; i < nodeIds.size(); ++i)
        morphologyMap[morphologies[i]].push_back(i);

    brayns::Boxf fastBounds;
    fastBounds.reset();

    const auto morphologyPipeline = createMorphologyPipeline(popProps);

    for(const auto& entry : morphologyMap)
    {
        // Load morphology
        const auto morphPath = populationSettings.morphologiesDir + "/" + entry.first + ".swc";
        const auto builder = morphologyPipeline.importMorphology(morphPath,
                                                                 popProps.morphologySections);

        // Instantiate the morphology for every cell with such morphology class
        for(const auto idx : entry.second)
        {
            // Build cell morphology geometry
            auto morphologyInstance = builder->instantiate(positions[idx], rotations[idx]);

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
            if(!mapping.empty())
            {
                const auto& cellMapping = mapping[cellIndex];
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
}
}
}
