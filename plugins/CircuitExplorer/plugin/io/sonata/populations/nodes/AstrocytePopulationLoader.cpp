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

#include "AstrocytePopulationLoader.h"

#include <plugin/io/sonata/data/SonataCells.h>

#include <plugin/io/sonata/SonataFactory.h>
#include <plugin/io/sonata/morphology/neuron/NeuronMorphology.h>
#include <plugin/io/sonata/morphology/neuron/NeuronMorphologyPipeline.h>
#include <plugin/io/sonata/morphology/neuron/pipeline/RadiusMultiplier.h>
#include <plugin/io/sonata/morphology/neuron/pipeline/RadiusSmoother.h>

#include <common/log.h>

namespace
{
auto createMorphologyPipeline(const PopulationLoadConfig& loadSettings)
{
    NeuronMorphologyPipeline pipeline;
    if(loadSettings.radiusMultiplier != 1.f)
        pipeline.registerStage<RadiusMultiplier>(loadSettings.radiusMultiplier);

    if(loadSettings.neuronMode == "smooth")
        pipeline.registerStage<RadiusSmoother>();

    return pipeline;
}
} // namespace

std::vector<MorphologyInstancePtr>
AstrocytePopulationLoader::load(const PopulationLoadConfig& loadSettings,
                                const bbp::sonata::Selection& nodeSelection,
                                const brayns::LoaderProgress& updateCb) const
{
    const SonataFactories factories;

    const auto nodesSize = nodeSelection.flatSize();
    const auto morphologies = SonataCells::getMorphologies(_population, nodeSelection);
    const auto positions = SonataCells::getPositions(_population, nodeSelection);
    const brayns::Quaternion dummy;

    std::vector<MorphologyInstancePtr> result (nodesSize);

    // Maps morphology class to all cell indices that uses that class
    std::unordered_map<std::string, std::vector<size_t>> morphologyMap;
    for(size_t i = 0; i < nodesSize; ++i)
        morphologyMap[morphologies[i]].push_back(i);

    const auto morphologyPipeline = createMorphologyPipeline(loadSettings);

    PLUGIN_WARN << "Astrocytes hardcoded h5 morphology type" << std::endl;

    for(const auto& entry : morphologyMap)
    {
        // Load morphology
        const auto morphPath = _populationProperties.morphologiesDir + "/" + entry.first + ".h5";
        NeuronMorphology m (morphPath, loadSettings.neuronSections);
        morphologyPipeline.process(m);
        auto builder = factories.neuronBuilders().instantiate(loadSettings.neuronMode);
        builder->build(m);

        // Instantiate the morphology for every cell with such morphology class
        for(const auto idx : entry.second)
            result[idx] = builder->instantiate(positions[idx], dummy);
    }

    return result;
}
