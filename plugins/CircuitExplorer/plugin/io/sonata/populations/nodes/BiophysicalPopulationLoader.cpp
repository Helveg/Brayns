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

#include "BiophysicalPopulationLoader.h"

#include "../PopulationFactory.h"

#include "../../data/SonataCells.h"

#include "../../simulations/SonataSimulation.h"

#include "../../morphology/MorphologyPipeline.h"
#include "../../morphology/builders/SDFGeometryBuilder.h"
#include "../../morphology/pipeline/RadiusMultiplier.h"
#include "../../morphology/pipeline/RadiusSmoother.h"

#include <iostream>

namespace
{
class Registerer
{
public:
    Registerer()
    {
        PopulationFactory::instance()
                .registerNodeLoader<BiophysicalPopualtionLoader>("biophysical");
    }
};
Registerer registerer;

auto createMorphologyPipeline(const PopulationLoadConfig& loadSettings)
{
    MorphologyPipeline pipeline;
    if(loadSettings.morphologyRadius != 1.f)
        pipeline.registerStage<morphology::RadiusMultiplier>(loadSettings.morphologyRadius);

    if(loadSettings.morphologyMode == "smooth")
        pipeline.registerStage<morphology::RadiusSmoother>();

    return pipeline;
}
}

std::vector<MorphologyInstancePtr>
BiophysicalPopualtionLoader::load(const PopulationLoadConfig& loadSettings,
                                  const bbp::sonata::Selection& nodeSelection,
                                  const brayns::LoaderProgress& updateCb) const
{
    const auto nodesSize = nodeSelection.flatSize();
    const auto morphologies = SonataCells::getMorphologies(_population, nodeSelection);
    const auto positions = SonataCells::getPositions(_population, nodeSelection);
    const auto rotations = SonataCells::getRotations(_population, nodeSelection);

    std::vector<MorphologyInstancePtr> result (nodesSize);

    // Maps morphology class to all cell indices that uses that class
    std::unordered_map<std::string, std::vector<size_t>> morphologyMap;
    for(size_t i = 0; i < nodesSize; ++i)
        morphologyMap[morphologies[i]].push_back(i);

    const auto morphologyPipeline = createMorphologyPipeline(loadSettings);

    for(const auto& entry : morphologyMap)
    {
        // Load morphology
        const auto morphPath = _populationProperties.morphologiesDir + "/" + entry.first + ".swc";
        const auto builder = morphologyPipeline
                                .createMorphologyBuilder(loadSettings.morphologyMode,
                                                         morphPath,
                                                         loadSettings.morphologySections);

        // Instantiate the morphology for every cell with such morphology class
        for(const auto idx : entry.second)
            result[idx] = builder->instantiate(positions[idx], rotations[idx]);
    }

    return result;
}
