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

#include "../SonataLoaderTypes.h"

#include <string>
#include <vector>

#include <bbp/sonata/population.h>

#include <brayns/common/simulation/AbstractSimulationHandler.h>

struct CellMapping
{
    size_t globalOffset;
    std::vector<uint16_t> offsets;
    std::vector<uint16_t> compartments;
};

class SonataSimulation
{
public:
    SonataSimulation(const std::string& path, const std::string& population)
     : _path(path)
     , _population(population)
    {
    }

    virtual ~SonataSimulation() = default;

    virtual std::vector<CellMapping> loadMapping(const bbp::sonata::Selection&) const = 0;

    virtual brayns::AbstractSimulationHandlerPtr
    createSimulationHandler(const bbp::sonata::Selection&) const = 0;

protected:
    const std::string _path;
    const std::string _population;
};

using SonataSimulationPtr = std::unique_ptr<SonataSimulation>;
