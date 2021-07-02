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

#include <string>
#include <vector>

#include <bbp/sonata/report_reader.h>

#include <brayns/common/simulation/AbstractSimulationHandler.h>

namespace sonata
{
namespace data
{
enum class SimulationType : uint8_t
{
    NONE = 0,
    SPIKES = 1,
    REPORT = 2
};

struct SimulationMapping
{
    size_t globalOffset;
    std::vector<uint16_t> sectionsOffsets;
    std::vector<uint16_t> sectionsCompartments;
};

class SonataSimulationMapping
{
public:
    SonataSimulationMapping(const std::string& filePath,
                            const std::string& populationName,
                            const bbp::sonata::Selection& selection)
     : _filePath(filePath)
     , _population(populationName)
     , _selection(selection)
    {
    }

    virtual std::vector<SimulationMapping> compute() const = 0;

    virtual brayns::AbstractSimulationHandlerPtr createSimulationHandler() const = 0;

protected:
    const std::string _filePath;
    const std::string _population;
    const bbp::sonata::Selection _selection;
};

class ReportMapping : public SonataSimulationMapping
{
public:
    ReportMapping(const std::string& filePath,
                  const std::string& populationName,
                  const bbp::sonata::Selection& selection)
     : SonataSimulationMapping(filePath, populationName, selection)
    {
    }

    std::vector<SimulationMapping> compute() const final;

    brayns::AbstractSimulationHandlerPtr createSimulationHandler() const final;
};

class SpikeMapping : public SonataSimulationMapping
{
public:
    SpikeMapping(const std::string& filePath,
                 const std::string& populationName,
                 const bbp::sonata::Selection& selection)
     : SonataSimulationMapping(filePath, populationName, selection)
    {
    }

    std::vector<SimulationMapping> compute() const final;

    brayns::AbstractSimulationHandlerPtr createSimulationHandler() const final;
};
}
}
