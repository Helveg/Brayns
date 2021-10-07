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

#include "EndFootPopulationLoader.h"

#include <plugin/api/Log.h>
#include <plugin/io/sonataloader/data/SonataEndFeetReader.h>
#include <plugin/io/sonataloader/data/SonataSelection.h>
#include <plugin/io/sonataloader/data/SonataSynapses.h>
#include <plugin/io/sonataloader/populations/edges/colorhandlers/EndFootColorHandler.h>
#include <plugin/io/synapse/groups/EndFootGroup.h>

#include <nlohmann/json.hpp>

#include <brayns/common/utils/filesystem.h>

namespace sonataloader
{
namespace
{
// The endfeet mesh file is not retruned by bbp::sonata::CircuitConfig (by now).
// Get it manually from the expanded json
std::string __getEndFeetAreasPath(const bbp::sonata::CircuitConfig& config,
                                  const std::string& edgePopulation,
                                  const std::string& basePath)
{
    const auto json = nlohmann::json::parse(config.getExpandedJSON());

    std::string resultPath = "";

    // First fetch default one, if any
    const auto componentsIt = json.find("components");
    if(componentsIt != json.end())
    {
        const auto& componentsJson = *componentsIt;
        auto endFeetIt = componentsJson.find("end_feet_area");
        if(endFeetIt != componentsJson.end())
            resultPath = endFeetIt->get<std::string>();
    }


    const auto& edgeNetwork = json["networks"]["edges"];
    auto it = edgeNetwork.begin();
    bool found = false;
    for(; it != edgeNetwork.end() && !found; ++it)
    {
        auto file = (*it)["edges_file"].get<std::string>();
        if(file[0] != '/')
            file = fs::absolute(fs::path(basePath) / fs::path(file)).string();

        const auto populationStorage = bbp::sonata::EdgeStorage(file);
        for(const auto& population : populationStorage.populationNames())
        {
            if(population != edgePopulation)
                continue;

            found = true;

            auto popIt = it->find("populations");
            if(popIt != it->end())
            {
                auto edgePopIt = popIt->find(edgePopulation);
                if(edgePopIt != popIt->end())
                {
                    auto edgePopEndFeetIt = edgePopIt->find("end_feet_area");
                    if(edgePopEndFeetIt != edgePopIt->end())
                        resultPath = edgePopEndFeetIt->get<std::string>();
                }
            }
            break;
        }
    }

    if(resultPath.empty())
        throw std::runtime_error("EndFootPopulationLoader: Cannot locate endfeet areas H5 file");
    else if(!fs::path(resultPath).is_absolute())
        resultPath = fs::path(fs::path(basePath)/fs::path(resultPath)).lexically_normal().string();

    return resultPath;
}
}

std::vector<std::unique_ptr<SynapseGroup>>
EndFootPopulationLoader::load(const PopulationLoadConfig& loadConfig,
                              const bbp::sonata::Selection& nodeSelection,
                              SubProgressReport& cb) const
{
    if(_afferent)
        throw std::runtime_error("Afferent edges not supported on endfoot connectivity");

    const auto basePath = fs::path(loadConfig.configPath).parent_path().string();
    auto path = __getEndFeetAreasPath(_config, _population.name(), basePath);

    const auto nodes = nodeSelection.flatten();

    const auto edgeSelection = EdgeSelection(_population.efferentEdges(nodes))
            .intersection(_percentage);
    const auto flatEdges = edgeSelection.flatten();
    const auto sourceNodes = SonataSynapses::getSourceNodes(_population, edgeSelection);
    const auto endFeetIds = SonataSynapses::getEndFeetIds(_population, edgeSelection);
    const auto endFeetPos = SonataSynapses::getEndFeetSurfacePos(_population, edgeSelection);

    auto meshes = SonataEndFeetReader::readEndFeet(path, endFeetIds, endFeetPos);

    // Initialize for every node, so the flat result will have a group for every node
    // (even if its empty, which allows to simply use vectors)
    std::map<uint64_t, std::unique_ptr<SynapseGroup>> mapping;
    for(const auto nodeId : nodes)
        mapping[nodeId] = std::make_unique<EndFootGroup>();

    // Group endfeet by the node id they belong to
    for(size_t i = 0; i < endFeetIds.size(); ++i)
    {
        EndFootGroup& group = static_cast<EndFootGroup&>(*mapping[sourceNodes[i]].get());
        group.addSynapse(endFeetIds[i], std::move(meshes[i]));
    }

    // Flatten
    std::vector<std::unique_ptr<SynapseGroup>> result (nodes.size());
    for(size_t i = 0; i < nodes.size(); ++i)
    {
        result[i] = std::move(mapping[nodes[i]]);
        cb.tick();
    }

    return result;
}

std::unique_ptr<CircuitColorHandler>
EndFootPopulationLoader::createColorHandler(brayns::ModelDescriptor *model,
                                            const std::string& config) const noexcept
{
    return std::make_unique<EndFootColorHandler>(model, config, _population.name(), _afferent);
}
}
