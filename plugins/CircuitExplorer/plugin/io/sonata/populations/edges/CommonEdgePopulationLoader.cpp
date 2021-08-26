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

#include "CommonEdgePopulationLoader.h"

#include <plugin/io/sonata/data/SonataSynapses.h>
#include <plugin/io/sonata/synapse/groups/SurfaceSynapseGroup.h>

#include <common/log.h>

std::vector<std::unique_ptr<SynapseGroup>>
CommonEdgePopulationLoader::load(const PopulationLoadConfig& loadConfig,
                                 const bbp::sonata::Selection& nodeSelection,
                                 const bool afferent) const
{
    const auto baseNodeList = nodeSelection.flatten();

    // Fill it by mapping node ID to synapse list in case there is a node Id without synapses,
    // so we can still place an empty vector at the end
    std::map<uint64_t, std::unique_ptr<SynapseGroup>> mapping;
    for(const auto nodeId : baseNodeList)
        mapping[nodeId] = std::make_unique<SurfaceSynapseGroup>();

    std::vector<uint64_t> srcNodes;
    std::vector<int32_t> sectionIds;
    std::vector<float> distances;
    std::vector<brayns::Vector3f> surfacePos;
    std::vector<uint64_t> edgeIds;

    if(afferent)
    {
        const auto edgeSelection = _population.afferentEdges(baseNodeList);
        edgeIds = edgeSelection.flatten();
        srcNodes = SonataSynapses::getAfferentTargetNodes(_population, edgeSelection);
        sectionIds = SonataSynapses::getAfferentSectionIds(_population, edgeSelection);
        surfacePos = SonataSynapses::getAfferentSurfacePos(_population, edgeSelection);
        distances = SonataSynapses::getAfferentSectionDistances(_population, edgeSelection);
    }
    else
    {
        const auto edgeSelection = _population.efferentEdges(baseNodeList);
        edgeIds = edgeSelection.flatten();
        srcNodes = SonataSynapses::getEfferentSourceNodes(_population, edgeSelection);
        surfacePos = SonataSynapses::getEfferentSurfacePos(_population, edgeSelection);
        sectionIds = SonataSynapses::getEfferentSectionIds(_population, edgeSelection);
        distances = SonataSynapses::getEfferentSectionDistances(_population, edgeSelection);
    }

    if(srcNodes.size() != sectionIds.size() || srcNodes.size() != surfacePos.size()
            || srcNodes.size() != distances.size())
        throw std::runtime_error("Edge population '" + _population.name()
                                 + "' attributes missmatch in size");

    // Group data by node id
    for(size_t i = 0; i < srcNodes.size(); ++i)
    {
        auto& buffer = static_cast<SurfaceSynapseGroup&>(*mapping[srcNodes[i]].get());
        buffer.addSynapse(edgeIds[i], sectionIds[i], distances[i], surfacePos[i]);
    }

    // Flatten
    std::vector<std::unique_ptr<SynapseGroup>> synapses (baseNodeList.size());
    for(size_t i = 0; i < baseNodeList.size(); ++i)
    {
        auto it = mapping.find(baseNodeList[i]);
        if(it != mapping.end())
            synapses[i] = std::move(it->second);
    }

    return synapses;
}
