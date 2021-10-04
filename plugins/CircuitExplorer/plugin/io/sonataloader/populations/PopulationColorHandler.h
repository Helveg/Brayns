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

#include <plugin/api/CircuitColorHandler.h>

#include <bbp/sonata/config.h>

namespace sonataloader
{
/**
 * @brief The PopulationColorHandler class is an abstract implementation of CircuitColorHandler
 *        that eases the final implementation for SONATA population circuits
 */
class PopulationColorHandler : public CircuitColorHandler
{
public:
    PopulationColorHandler(brayns::ModelDescriptor* model,
                           const std::string& configPath,
                           const std::string& population);

protected:
    const bbp::sonata::CircuitConfig _config;
    const std::string _population;
};

/**
 * @brief The EdgePopulationColorHandler class is an abstract implementation of a
 *        PopulationColorHandler, concretizing it a bit further for the final
 *        implementation of SONATA population synapse circuits
 */
class EdgePopulationColorHandler : public PopulationColorHandler
{
public:
    EdgePopulationColorHandler(brayns::ModelDescriptor* model,
                               const std::string& configPath,
                               const std::string& population,
                               const bool afferent);

protected:
    const bool _afferent;
    const std::string _nodePopulation;
};
}