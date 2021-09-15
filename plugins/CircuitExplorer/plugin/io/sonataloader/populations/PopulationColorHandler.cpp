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

#include "PopulationColorHandler.h"

namespace sonataloader
{
namespace
{
inline std::string __getNodePopulation(const bbp::sonata::CircuitConfig& config,
                                       const std::string& edgePopulation,
                                       const bool afferent)
{
    const auto edges = config.getEdgePopulation(edgePopulation);
    return afferent? edges.target() : edges.source();
}
}

PopulationColorHandler::PopulationColorHandler(brayns::ModelDescriptor* model,
                                               const std::string& configPath,
                                               const std::string& population)
 : CircuitColorHandler(model)
 , _config(bbp::sonata::CircuitConfig::fromFile(configPath))
 , _population(population)
{
}


EdgePopulationColorHandler::EdgePopulationColorHandler(brayns::ModelDescriptor* model,
                                                       const std::string& configPath,
                                                       const std::string& population,
                                                       const bool afferent)
 : PopulationColorHandler(model, configPath, population)
 , _afferent(afferent)
 , _nodePopulation(__getNodePopulation(_config, population, afferent))
{
}
}
