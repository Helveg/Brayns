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

#include <brayns/common/utils/filesystem.h>

#include <plugin/api/Log.h>
#include <plugin/io/morphology/neuron/NeuronMorphology.h>
#include <plugin/io/morphology/neuron/NeuronMorphologyPipeline.h>
#include <plugin/io/morphology/neuron/pipeline/RadiusMultiplier.h>
#include <plugin/io/morphology/neuron/pipeline/RadiusOverride.h>
#include <plugin/io/morphology/neuron/pipeline/RadiusSmoother.h>
#include <plugin/io/sonataloader/SonataFactory.h>
#include <plugin/io/sonataloader/data/SonataCells.h>
#include <plugin/io/sonataloader/populations/nodes/colorhandlers/NeuronColorHandler.h>

#include <future>
#include <mutex>

namespace sonataloader
{
namespace
{
auto __createMorphologyPipeline(const NeuronLoadConfig& loadSettings)
{
    NeuronMorphologyPipeline pipeline;
    if(loadSettings.radiusOverride > 0.f)
        pipeline.registerStage<RadiusOverride>(loadSettings.radiusOverride);
    else
    {
        if(loadSettings.radiusMultiplier != 1.f)
            pipeline.registerStage<RadiusMultiplier>(loadSettings.radiusMultiplier);

        // Dont apply smoothing if we are loading only soma...
        if(loadSettings.sections != NeuronSection::SOMA &&
                loadSettings.mode == NeuronGeometryType::SMOOTH)
            pipeline.registerStage<RadiusSmoother>();
    }
    return pipeline;
}
} // namespace

std::vector<MorphologyInstancePtr>
AstrocytePopulationLoader::load(const PopulationLoadConfig& loadSettings,
                                const bbp::sonata::Selection& nodeSelection,
                                SubProgressReport& cb) const
{
    static std::mutex loadMtx;

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

    const auto morphologyPipeline = __createMorphologyPipeline(loadSettings.neurons);

    PLUGIN_WARN << "Astrocytes hardcoded h5 morphology type" << std::endl;

    const auto loadFn = [&](const std::string& name, const std::vector<size_t>& indices)
    {
        std::string morphPath = _populationProperties.morphologiesDir + "/" + name + ".swc";
        if(!fs::exists(morphPath))
            morphPath = _populationProperties.morphologiesDir + "/" + name + ".h5";

        NeuronMorphology morphology(morphPath, loadSettings.neurons.sections);
        morphologyPipeline.process(morphology);
        auto builder = factories.neuronBuilders().instantiate(loadSettings.neurons.mode);
        builder->build(morphology);
        for(const auto idx : indices)
            result[idx] = builder->instantiate(positions[idx], dummy);
        {
            std::lock_guard<std::mutex> lock(loadMtx);
            cb.tickBatch(indices.size());
        }
    };

    std::vector<std::future<void>> loadTasks;
    loadTasks.reserve(morphologyMap.size());
    for(const auto& entry : morphologyMap)
        loadTasks.push_back(std::async(loadFn, entry.first, entry.second));

    for(const auto& task : loadTasks)
    {
        if(task.valid())
            task.wait();
    }

    return result;
}

std::unique_ptr<CircuitColorHandler>
AstrocytePopulationLoader::createColorHandler(brayns::ModelDescriptor* model,
                                              const std::string& config) const noexcept
{
    return std::make_unique<NeuronColorHandler>(model, config, _population.name());
}
}
