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

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <bbp/sonata/population.h>

#include <brayns/common/simulation/AbstractSimulationHandler.h>

namespace sonataloader
{
struct NodeSimulationMapping
{
    size_t globalOffset;
    std::vector<uint16_t> offsets;
    std::vector<uint16_t> compartments;
};

struct EdgeSimulationMapping
{
    std::unordered_map<bbp::sonata::EdgeID, uint64_t> offsets;
};

template<typename MappingType>
class SimulationLoader
{
public:
    SimulationLoader(const std::string& path, const std::string& population)
     : _path(path)
     , _population(population)
    {
    }

    virtual ~SimulationLoader() = default;

    virtual std::vector<MappingType>
    loadMapping(const bbp::sonata::Selection&) const = 0;

    virtual brayns::AbstractSimulationHandlerPtr
    createSimulationHandler(const bbp::sonata::Selection&) const = 0;

protected:
    const std::string _path;
    const std::string _population;
};

template<typename MappingType>
using SimulationLoaderPtr = std::unique_ptr<SimulationLoader<MappingType>>;

using NodeSimulationLoaderPtr = SimulationLoaderPtr<NodeSimulationMapping>;
using EdgeSimulationLoaderPtr = SimulationLoaderPtr<EdgeSimulationMapping>;
}
