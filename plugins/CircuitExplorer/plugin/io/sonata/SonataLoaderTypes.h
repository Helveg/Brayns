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

#include <cstdint>
#include <string>

enum class SimulationType : uint8_t
{
    NONE = 0,
    SPIKES = 1,
    REPORT = 2,
    SUMMATION = 3,
    SYNAPSE = 4,
    BLOODFLOW = 5
};

inline std::string simulationTypeToString(const SimulationType type) noexcept
{
    switch(type)
    {
    case SimulationType::SPIKES:
        return "Spikes";
    case SimulationType::REPORT:
        return "Compartment";
    case SimulationType::SUMMATION:
        return "Summation";
    case SimulationType::SYNAPSE:
        return "Synapse";
    case SimulationType::BLOODFLOW:
        return "Bloodflow";
    default:
        return "Unknown";
    }
}

enum class MorphologySection : uint8_t
{
    ALL = 0,
    SOMA = 1,
    AXON = 2,
    DENDRITE = 3,
    APICAL_DENDRITE = 4
};

enum class VasculatureSection : uint8_t
{
    ALL = 0,
    VEIN = 1,
    ARTERY = 2,
    VENULE = 3,
    ARTERIOLE = 4,
    VENOUS_CAPILLARY = 5,
    ARTERIAL_CAPILLARY = 6,
    TRANSITIONAL = 7
};
