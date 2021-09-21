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

#include <plugin/io/util/EnumWrapper.h>

#include <brayns/common/utils/stringUtils.h>

namespace
{
constexpr char VANILLA_NAME[] = "vanilla";
constexpr char SMOOTH_NAME[]  = "smooth";
constexpr char SAMPLES_NAME[] = "samples";
}

/**
 * @brief The NeuronGeometryType enum holds the list of available geometry types into which
 *        a morphology can be transformed
 */
enum class NeuronGeometryType : uint8_t
{
    VANILLA = 0,
    SMOOTH = 1,
    SAMPLES = 2
};

template<>
inline std::vector<std::string> EnumWrapper<NeuronGeometryType>::toStringList() const noexcept
{
    return {VANILLA_NAME, SMOOTH_NAME, SAMPLES_NAME};
}

template<>
inline EnumWrapper<NeuronGeometryType>::Types
EnumWrapper<NeuronGeometryType>::fromString(const std::string& src) const
{
    const auto srcLc = brayns::string_utils::toLowercase(src);
    if(src == VANILLA_NAME)
        return NeuronGeometryType::VANILLA;
    else if(src == SMOOTH_NAME)
        return NeuronGeometryType::SMOOTH;
    else if(src == SAMPLES_NAME)
        return NeuronGeometryType::SAMPLES;

    throw std::invalid_argument("NeuronGeometryType: Unknown geometry type name '" + src + "'");
}

template<>
inline std::string
EnumWrapper<NeuronGeometryType>::toString(
        const EnumWrapper<NeuronGeometryType>::Types type) const noexcept
{
    if(type == Types::VANILLA)
        return VANILLA_NAME;
    else if(type == Types::SMOOTH)
        return SMOOTH_NAME;
    else if(type == Types::SAMPLES)
        return SAMPLES_NAME;

    return "";
}
