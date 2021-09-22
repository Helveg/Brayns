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
/**
 * @brief The NodeSimulationMapping struct holds information about a single cells simulation
 *        mapping. The mapping is given as a global offset into the simulation array, plus
 *        local offsets and number of compartments for each reported section
 */
struct NodeSimulationMapping
{
    size_t globalOffset;
    std::vector<uint16_t> offsets;
    std::vector<uint16_t> compartments;
};

/**
 * @brief The EdgeSimulationMapping struct holds the simulation mapping information of all
 *        the edges of a single cell. The mapping is given as the simulation array offset
 *        for each edge id.
 */
struct EdgeSimulationMapping
{
    std::unordered_map<bbp::sonata::EdgeID, uint64_t> offsets;
};

/**
 * @brief The SimulationLoader class is the base class to manage SONATA report simulations.
 *        Is in charge of computing the mapping for a set of nodes, and create the appropiate
 *        type of brayns::AbstractSimulationHandler for the type of population it handles
 */
template<typename MappingType>
class SimulationLoader
{
public:
    /**
     * @brief initialization with the path to the report file and the population
     *        to look for inside of it
     */
    SimulationLoader(const std::string& path, const std::string& population)
     : _path(path)
     , _population(population)
    {
    }

    virtual ~SimulationLoader() = default;

    /**
     * @brief computes and returns the simulation mapping for the set of given
     *        cells
     */
    virtual std::vector<MappingType>
    loadMapping(const bbp::sonata::Selection&) const = 0;

    /**
     * @brief creates a brayns::AbstractSimulationHandler instance appropriate for
     *        the type of nodes this simulation is reporting on
     */
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
