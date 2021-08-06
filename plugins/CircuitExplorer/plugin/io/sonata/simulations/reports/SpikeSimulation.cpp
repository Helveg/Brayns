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

#include "SpikeSimulation.h"
#include "../SonataSimulationFactory.h"
#include "../handlers/SonataSpikeHandler.h"

#include <bbp/sonata/report_reader.h>

namespace
{
class Registerer
{
public:
    Registerer()
    {
        SonataSimulationFactory::instance()
                .registerSimulation<SpikeSimulation>(SimulationType::SPIKES);
    }
};
Registerer registerer;
}


SpikeSimulation::SpikeSimulation(const std::string& path, const std::string& population)
 : SonataSimulation(path, population)
{
    const bbp::sonata::SpikeReader reader (path);
    const auto pops = reader.getPopulationNames();

    auto it = std::find_if(pops.begin(), pops.end(), [&](const std::string& p)
    {
        return p == population;
    });

    if(it == pops.end())
        throw std::runtime_error("Spike simulation " + path + " does not have a population "
                                 "'" + population + "'");
}

std::vector<CellMapping> SpikeSimulation::loadMapping(const bbp::sonata::Selection& s) const
{
    std::vector<CellMapping> mapping (s.flatSize());
    for(size_t i = 0; i < mapping.size(); ++i)
        mapping[i].globalOffset = i;
    return mapping;
}

brayns::AbstractSimulationHandlerPtr
SpikeSimulation::createSimulationHandler(const bbp::sonata::Selection& s) const
{
    return std::make_shared<SonataSpikeHandler>(_path, _population, s);
}
