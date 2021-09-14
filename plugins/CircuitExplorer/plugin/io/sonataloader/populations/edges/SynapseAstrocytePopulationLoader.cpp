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

#include "SynapseAstrocytePopulationLoader.h"

#include <plugin/io/sonataloader/data/SonataSelection.h>
#include <plugin/io/sonataloader/data/SonataSynapses.h>
#include <plugin/io/sonataloader/populations/edges/colorhandlers/CommonEdgeColorHandler.h>
#include <plugin/io/synapse/groups/SynapseAstrocyteGroup.h>

namespace sonataloader
{
std::vector<std::unique_ptr<SynapseGroup>>
SynapseAstrocytePopulationLoader::load(const PopulationLoadConfig& loadConfig,
                                       const bbp::sonata::Selection& nodeSelection,
                                       SubProgressReport& cb) const
{
    if(_afferent)
        throw std::runtime_error("synapse_astrocyte population should have been splitted at loading. "
                                 "This should not have happened");

    const auto baseNodeList = nodeSelection.flatten();

    // Fill it by mapping node ID to synapse list in case there is a node Id without synapses,
    // so we can still place an empty vector at the end
    std::map<uint64_t, std::unique_ptr<SynapseGroup>> mapping;
    for(const auto nodeId : baseNodeList)
        mapping[nodeId] = std::make_unique<SynapseAstrocyteGroup>();

    const auto edgeSelection = EdgeSelection(_population.efferentEdges(baseNodeList))
            .intersection(_percentage);
    const auto edgeIds = edgeSelection.flatten();
    const auto srcNodes = SonataSynapses::getSourceNodes(_population, edgeSelection);
    const auto sectionIds = SonataSynapses::getEfferentAstrocyteSectionIds(_population,
                                                                           edgeSelection);
    const auto distances = SonataSynapses::getEfferentAstrocyteSectionDistances(_population,
                                                                                edgeSelection);

    if(srcNodes.size() != sectionIds.size() || srcNodes.size() != distances.size())
        throw std::runtime_error("Edge population '" + _population.name()
                                 + "' attributes missmatch in size");

    // Group data by node id
    for(size_t i = 0; i < srcNodes.size(); ++i)
    {
        auto& buffer = static_cast<SynapseAstrocyteGroup&>(*mapping[srcNodes[i]].get());
        buffer.addSynapse(edgeIds[i], sectionIds[i], distances[i]);
    }

    // Flatten
    std::vector<std::unique_ptr<SynapseGroup>> synapses (baseNodeList.size());
    for(size_t i = 0; i < baseNodeList.size(); ++i)
    {
        auto it = mapping.find(baseNodeList[i]);
        if(it != mapping.end())
            synapses[i] = std::move(it->second);
        cb.tick();
    }

    return synapses;
}

std::unique_ptr<CircuitColorHandler>
SynapseAstrocytePopulationLoader::createColorHandler(brayns::ModelDescriptor *model,
                                                     const std::string& config) const noexcept
{
    return std::make_unique<CommonEdgeColorHandler>(model, config, _population.name(), _afferent);
}
}
