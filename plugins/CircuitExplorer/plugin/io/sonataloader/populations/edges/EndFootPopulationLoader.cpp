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
std::string __getEndFeetAreasPath(const bbp::sonata::CircuitConfig& config,
                                  const std::string& edgePopulation,
                                  const std::string& basePath)
{
    const auto json = nlohmann::json::parse(config.getExpandedJSON());

    const auto& edgeNetwork = json["networks"]["edges"];
    auto it = edgeNetwork.begin();
    for(; it != edgeNetwork.end(); ++it)
    {
        auto file = (*it)["edges_file"].get<std::string>();
        if(file[0] != '/')
            file = fs::absolute(fs::path(basePath) / fs::path(file)).string();

        const auto populationStorage = bbp::sonata::EdgeStorage(file);
        for(const auto& population : populationStorage.populationNames())
        {
            if(population == edgePopulation && fs::exists(file))
                return fs::path(file).parent_path().string();
        }
    }

    throw std::runtime_error("EndFootPopulationLoader: Cannot locate endfeet areas H5 file");
}
}

std::vector<std::unique_ptr<SynapseGroup>>
EndFootPopulationLoader::load(const PopulationLoadConfig& loadConfig,
                              const bbp::sonata::Selection& nodeSelection,
                              SubProgressReport& cb) const
{
    if(_afferent)
        throw std::runtime_error("Afferent edges not supported on endfoot connectivity");

    PLUGIN_WARN << "CURRENTLY obtaining the endfeet_areas file is hardcoded" << std::endl;

    const auto basePath = fs::path(loadConfig.configPath).parent_path().string();
    auto path = __getEndFeetAreasPath(_config, _population.name(), basePath) + "/endfeet_areas.h5";

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
