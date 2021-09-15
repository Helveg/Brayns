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

#include <cstdint>
#include <string>

#include <brayns/common/utils/stringUtils.h>

#include <plugin/io/util/EnumWrapper.h>

namespace sonataloader
{
enum class SimulationType : uint8_t
{
    NONE = 0,
    SPIKES = 1,
    COMPARTMENT = 2,
    SUMMATION = 3,
    SYNAPSE = 4,
    BLOODFLOW_PRESSURE = 5,
    BLOODFLOW_SPEED = 6,
    BLOODFLOW_RADII = 7
};
}

namespace
{
constexpr char NONE_NAME[]                  = "none";
constexpr char SPIKES_NAME[]                = "spikes";
constexpr char COMPARTMENT_NAME[]           = "compartment";
constexpr char SUMMATION_NAME[]             = "summation";
constexpr char SYNAPSE_NAME[]               = "synapse";
constexpr char BLOODFLOW_PRESSURE_NAME[]    = "bloodflow pressure";
constexpr char BLOODFLOW_SPEED_NAME[]       = "bloodflow speed";
constexpr char BLOODFLOW_RADII_NAME[]       = "bloodflow radii";
}

template<>
inline std::vector<std::string>
EnumWrapper<sonataloader::SimulationType>::toStringList() const noexcept
{
    return {NONE_NAME, SPIKES_NAME, COMPARTMENT_NAME,
            SUMMATION_NAME, SYNAPSE_NAME, BLOODFLOW_PRESSURE_NAME,
            BLOODFLOW_SPEED_NAME, BLOODFLOW_RADII_NAME};
}

template<>
inline EnumWrapper<sonataloader::SimulationType>::Types
EnumWrapper<sonataloader::SimulationType>::fromString(const std::string& src) const
{
    const auto srcLc = brayns::string_utils::toLowercase(src);

    if(src == SPIKES_NAME)
        return Types::SPIKES;
    else if(src == COMPARTMENT_NAME)
        return Types::COMPARTMENT;
    else if(src == SUMMATION_NAME)
        return Types::SUMMATION;
    else if(src == SYNAPSE_NAME)
        return Types::SYNAPSE;
    else if(src == BLOODFLOW_PRESSURE_NAME)
        return Types::BLOODFLOW_PRESSURE;
    else if(src == BLOODFLOW_SPEED_NAME)
        return Types::BLOODFLOW_SPEED;
    else if(src == BLOODFLOW_RADII_NAME)
        return Types::BLOODFLOW_RADII;
    else if(src == NONE_NAME)
        return Types::NONE;

    throw std::invalid_argument("SimulationType: Unknown section name '" + src + "'");
}

template<>
inline std::string
EnumWrapper<sonataloader::SimulationType>::toString(
        const EnumWrapper<sonataloader::SimulationType>::Types type) const noexcept
{
    if(type == Types::SPIKES)
        return SPIKES_NAME;
    else if(type == Types::COMPARTMENT)
        return COMPARTMENT_NAME;
    else if(type == Types::SUMMATION)
        return SUMMATION_NAME;
    else if(type == Types::SYNAPSE)
        return SYNAPSE_NAME;
    else if(type == Types::BLOODFLOW_PRESSURE)
        return BLOODFLOW_PRESSURE_NAME;
    else if(type == Types::BLOODFLOW_SPEED)
        return BLOODFLOW_SPEED_NAME;
    else if(type == Types::BLOODFLOW_RADII)
        return BLOODFLOW_RADII_NAME;
    else if(type == Types::NONE)
        return NONE_NAME;

    return "";
}
