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

namespace bbploader
{
/**
 * @brief The SimulationType enum list all supported simulation types from BBP internat formats
 */
enum class SimulationType : uint8_t
{
    NONE = 0,
    SPIKES = 1,
    COMPARTMENT = 2,
};
}

namespace
{
constexpr char NONE_NAME[]          = "none";
constexpr char SPIKES_NAME[]        = "spikes";
constexpr char COMPARTMENT_NAME[]   = "compartment";
}

template<>
inline std::vector<std::string>
EnumWrapper<bbploader::SimulationType>::toStringList() const noexcept
{
    return {NONE_NAME, SPIKES_NAME, COMPARTMENT_NAME};
}

template<>
inline EnumWrapper<bbploader::SimulationType>::Types
EnumWrapper<bbploader::SimulationType>::fromString(const std::string& src) const
{
    const auto srcLc = brayns::string_utils::toLowercase(src);

    if(src == SPIKES_NAME)
        return Types::SPIKES;
    else if(src == COMPARTMENT_NAME)
        return Types::COMPARTMENT;
    else if(src == NONE_NAME)
        return Types::NONE;

    throw std::invalid_argument("SimulationType: Unknown section name '" + src + "'");
}

template<>
inline std::string
EnumWrapper<bbploader::SimulationType>::toString(
        const EnumWrapper<bbploader::SimulationType>::Types type) const noexcept
{
    if(type == Types::SPIKES)
        return SPIKES_NAME;
    else if(type == Types::COMPARTMENT)
        return COMPARTMENT_NAME;
    else if(type == Types::NONE)
        return NONE_NAME;

    return "";
}
