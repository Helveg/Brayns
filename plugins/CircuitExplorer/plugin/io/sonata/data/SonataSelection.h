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

#include <bbp/sonata/config.h>
#include <bbp/sonata/population.h>

#include "../SonataLoaderTypes.h"


/**
 * @brief The SonataSelection class is in charge of creating the selection of cells that
 *        will be loaded into brayns, taking into account user parameters and network
 *        data.
 */
class NodeSelection
{
public:
    NodeSelection();

    /**
     * @brief select node Ids from a node population based on input nodesets
     */
    void select(const bbp::sonata::CircuitConfig& config,
                const std::string& population,
                const std::vector<std::string>& nodeSets);

    /**
     * @brief select node ids from a node population based on a list of node Ids
     */
    void select(const std::vector<uint64_t>& nodeList);

    /**
     * @brief select node ids from a node population based on reported nodes in a simulation
     */
    void select(const SimulationType simType,
                const std::string& reportPath,
                const std::string& population);

    /**
     * @brief return the best selection candidate based on what was selected:
     *  - If a node list was provided, this is returned. If also a simulation was
     *    provided, the intersection of both is returned
     *  - If no list was provided, but a simulation was provided, the intersection
     *    between what was selected based on nodesets and the report node ids is
     *    returned
     */
    bbp::sonata::Selection intersection(const double percentage);

private:
    bbp::sonata::Selection _nodeSetsSelection;
    bbp::sonata::Selection _nodeListSelection;
    bbp::sonata::Selection _simulationSelection;
};

class EdgeSelection
{
public:
    EdgeSelection();

    void select(const bbp::sonata::CircuitConfig& config,
                const std::string& edgePopulation,
                const bbp::sonata::Selection& nodeSelection,
                const bool afferent);

    const bbp::sonata::Selection& selection() const noexcept;

private:
    bbp::sonata::Selection _selection;
};
