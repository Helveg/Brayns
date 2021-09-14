/* Copyright (c) 2015-2021, EPFL/Blue Brain Project
 * All rights reserved. Do not distribute without permission.
 * Responsible Author: Nadir Roman <nadir.romanguerrero@epfl.ch>
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

#include "CellLoader.h"

#include <plugin/io/bbploader/BBPLoaderFactory.h>
#include <plugin/io/morphology/neuron/NeuronMorphology.h>
#include <plugin/io/morphology/neuron/NeuronMorphologyPipeline.h>
#include <plugin/io/morphology/neuron/pipeline/RadiusMultiplier.h>
#include <plugin/io/morphology/neuron/pipeline/RadiusOverride.h>
#include <plugin/io/morphology/neuron/pipeline/RadiusSmoother.h>

namespace bbploader
{
namespace
{
inline NeuronMorphologyPipeline __createMorphologyPipeline(const BBPCircuitLoadConfig& lc)
{
    NeuronMorphologyPipeline pipeline;
    if(lc.radiusOverride > 0.f)
        pipeline.registerStage<RadiusOverride>(lc.radiusOverride);
    else
    {
        if(lc.radiusMultiplier != 1.f)
            pipeline.registerStage<RadiusMultiplier>(lc.radiusMultiplier);

        if(lc.geometryMode == NeuronGeometryType::SMOOTH &&
                lc.morphologySections != NeuronSection::SOMA)
            pipeline.registerStage<RadiusSmoother>();
    }

    return pipeline;
}
}

std::vector<MorphologyInstancePtr> CellLoader::load(const BBPCircuitLoadConfig& lc,
                                                    const brain::GIDSet& gids,
                                                    const brain::Circuit& circuit,
                                                    SubProgressReport& spr)
{
    const BBPLoaderFactory factory;

    const auto morphPaths = circuit.getMorphologyURIs(gids);
    std::unordered_map<std::string, std::vector<size_t>> morphPathMap;
    for(size_t i = 0; i < morphPaths.size(); ++i)
        morphPathMap[morphPaths[i].getPath()].push_back(i);

    const auto positions = circuit.getPositions(gids);
    const auto rotations = circuit.getRotations(gids);
    const auto pipeline = __createMorphologyPipeline(lc);
    std::vector<MorphologyInstancePtr> cells (gids.size());
    for(const auto& morphEntry : morphPathMap)
    {
        NeuronMorphology morphology(morphEntry.first, lc.morphologySections);
        pipeline.process(morphology);
        auto builder = factory.neuronBuilders().instantiate(lc.geometryMode);
        builder->build(morphology);
        for(const auto idx : morphEntry.second)
        {
            cells[idx] = builder->instantiate(positions[idx], rotations[idx]);
            spr.tick();
        }
    }

    return cells;
}
}
