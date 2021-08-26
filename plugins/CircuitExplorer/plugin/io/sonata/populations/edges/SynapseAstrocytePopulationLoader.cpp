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

#include <plugin/io/sonata/SonataFactory.h>
#include <plugin/io/sonata/data/SonataSynapses.h>
#include <plugin/io/sonata/synapse/groups/AggregateGroup.h>
#include <plugin/io/sonata/synapse/groups/SynapseAstrocyteGroup.h>

std::vector<std::unique_ptr<SynapseGroup>>
SynapseAstrocytePopulationLoader::load(const PopulationLoadConfig& loadConfig,
                                       const bbp::sonata::Selection& nodeSelection,
                                       const bool afferent) const
{
    const auto baseNodeList = nodeSelection.flatten();

    if(afferent)
    {
        // Fill it by mapping node ID to synapse list in case there is a node Id without synapses,
        // so we can still place an empty vector at the end
        std::map<uint64_t, std::unique_ptr<SynapseGroup>> mapping;
        for(const auto nodeId : baseNodeList)
            mapping[nodeId] = std::make_unique<SynapseAstrocyteGroup>();

        const auto edgeSelection = _population.afferentEdges(baseNodeList);
        const auto edgeIds = edgeSelection.flatten();
        const auto srcNodes = SonataSynapses::getAfferentAstrocyteSourceNodes(_population,
                                                                              edgeSelection);
        const auto sectionIds = SonataSynapses::getAfferentAstrocyteSectionIds(_population,
                                                                               edgeSelection);
        const auto distances = SonataSynapses::getAfferentAstrocyteSectionDistances(_population,
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
        }

        return synapses;
    }
    else
    {
        const auto edgeSelection = _population.efferentEdges(baseNodeList);

        std::vector<std::unique_ptr<SynapseGroup>> result (baseNodeList.size());
        for(size_t i = 0; i < baseNodeList.size(); ++i)
            result[i] = std::make_unique<AggregateGroup>();

        const auto populations = _population.getAttribute<std::string>("synapse_population",
                                                                       edgeSelection);
        const SonataFactories factories;

        for(const auto& population : populations)
        {
            const auto properties = _config.getEdgePopulationProperties(population);
            const auto edgeLoader = factories.edgeLoaders().instantiate(properties.type,
                                                                        _config,
                                                                        population,
                                                                        _percentage);

            auto synapses = edgeLoader->load(loadConfig, nodeSelection, false);
            for(size_t i = 0; i < synapses.size(); ++i)
            {
                auto& group = static_cast<AggregateGroup&>(*result[i].get());
                group.addGroup(population, std::move(synapses[i]));
            }
        }
        return result;
    }
}
