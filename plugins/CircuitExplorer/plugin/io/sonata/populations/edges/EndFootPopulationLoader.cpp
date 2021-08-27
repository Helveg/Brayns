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

#include <common/log.h>

#include <plugin/io/sonata/data/SonataEndFeetReader.h>
#include <plugin/io/sonata/data/SonataSynapses.h>
#include <plugin/io/sonata/synapse/groups/EndFootGroup.h>

#include <nlohmann/json.hpp>

#include <boost/filesystem.hpp>

namespace
{
std::string getEndFeetAreasPath(const bbp::sonata::CircuitConfig& config,
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
            file = boost::filesystem::absolute(boost::filesystem::path(file),
                                               boost::filesystem::path(basePath)).string();

        const auto populationStorage = bbp::sonata::EdgeStorage(file);
        for(const auto& population : populationStorage.populationNames())
        {
            if(population == edgePopulation && boost::filesystem::exists(file))
                return boost::filesystem::path(file).parent_path().string();
        }
    }

    throw std::runtime_error("EndFootPopulationLoader: Cannot locate endfeet areas H5 file");
}
}

std::vector<std::unique_ptr<SynapseGroup>>
EndFootPopulationLoader::load(const PopulationLoadConfig& loadConfig,
                              const bbp::sonata::Selection& nodeSelection,
                              const bool afferent) const
{
    if(!afferent)
        throw std::runtime_error("Efferent edges not supported on endfoot connectivity");

    PLUGIN_WARN << "CURRENTLY obtaining the endfeet_areas file is hardcoded" << std::endl;

    const auto basePath = boost::filesystem::path(loadConfig.configPath).parent_path().string();
    auto path = getEndFeetAreasPath(_config, _populationName, basePath) + "/endfeet_areas.h5";

    const auto nodes = nodeSelection.flatten();

    const auto edgeSelection = _population.afferentEdges(nodes);
    const auto flatEdges = edgeSelection.flatten();
    const auto sourceNodes = SonataSynapses::getSourceNodes(_population, edgeSelection);
    const auto endFeetIds = SonataSynapses::getEndFeetIds(_population, edgeSelection);
    const auto endFeetPos = SonataSynapses::getEndFeetSurfacePos(_population, edgeSelection);

    auto meshes = SonataEndFeetReader::readEndFeet(path, endFeetIds, endFeetPos);

    std::vector<std::unique_ptr<SynapseGroup>> result;
    result.push_back(std::make_unique<EndFootGroup>());
    EndFootGroup& group = static_cast<EndFootGroup&>(*result.back().get());
    for(size_t i = 0; i < endFeetIds.size(); ++i)
        group.addSynapse(endFeetIds[i], std::move(meshes[i]));

    return result;
}
