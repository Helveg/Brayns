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

#pragma once

#include "NodePopulationLoader.h"
#include "EdgePopulationLoader.h"

#include <unordered_map>

class PopulationFactory
{
public:
    static PopulationFactory& instance() noexcept
    {
        static PopulationFactory fact;
        return fact;
    }

    template<class T>
    void registerNodeLoader(const std::string& type)
    {
        static_assert (std::is_base_of<NodePopulationLoader, T>::value,
                       "Tried to register a non NodePopulationLoader");

        _nodes[type] = [](bbp::sonata::NodePopulation&& n,
                          bbp::sonata::PopulationProperties&& np)
        {
            return std::make_unique<T>(std::move(n), std::move(np));
        };
    }

    std::unique_ptr<NodePopulationLoader>
    createNodeLoader(bbp::sonata::NodePopulation&& nodes,
                     bbp::sonata::PopulationProperties&& properties)
    {
        auto it = _nodes.find(properties.type);
        if(it == _nodes.end())
            throw std::runtime_error("Non supported node population type: '"
                                     + properties.type + '"');

        return (it->second)(std::move(nodes), std::move(properties));
    }

    template<class T>
    void registerEdgeLoader(const std::string& type)
    {
        static_assert (std::is_base_of<EdgePopulationLoader, T>::value,
                       "Tried to register a non EdgePopulationLoader");

        _edges[type] = [](bbp::sonata::EdgePopulation&& e,
                          bbp::sonata::PopulationProperties&& ep)
        {
            return std::make_unique<T>(std::move(e), std::move(ep));
        };
    }

    std::unique_ptr<EdgePopulationLoader>
    createEdgeLoader(bbp::sonata::EdgePopulation&& edges,
                     bbp::sonata::PopulationProperties&& properties)
    {
        auto it = _edges.find(properties.type);
        if(it == _edges.end())
            throw std::runtime_error("Non supported edge population type: '"
                                     + properties.type + '"');

        return (it->second)(std::move(edges), std::move(properties));
    }

private:
    using NodeFactory = std::function<
                            NodePopulationLoaderPtr(
                                bbp::sonata::NodePopulation&&,
                                bbp::sonata::PopulationProperties&&)>;

    using EdgeFactory = std::function<
                            EdgePopulationLoaderPtr(
                                bbp::sonata::EdgePopulation&&,
                                bbp::sonata::PopulationProperties&&)>;


    std::unordered_map<std::string, NodeFactory> _nodes;
    std::unordered_map<std::string, EdgeFactory> _edges;
};

