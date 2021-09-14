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

#include <plugin/io/util/EnumWrapper.h>

#include <brayns/common/utils/stringUtils.h>

namespace
{
constexpr char SOMA_NAME[]              = "soma";
constexpr char AXON_NAME[]              = "axon";
constexpr char DENDRITE_NAME[]          = "basal dendrite";
constexpr char APICALDENDRITE_NAME[]    = "apical dendrite";
}

enum class NeuronSection : uint8_t
{
    NONE            = 0,
    SOMA            = 1,
    AXON            = 2,
    DENDRITE        = 4,
    APICAL_DENDRITE = 8,
    ALL = SOMA | AXON | DENDRITE | APICAL_DENDRITE
};

inline NeuronSection operator& (const NeuronSection a, const NeuronSection b) noexcept
{
    return static_cast<NeuronSection>(static_cast<uint8_t>(a)
                                      & static_cast<uint8_t>(b));
}

inline NeuronSection& operator&= (NeuronSection& a, const NeuronSection& b) noexcept
{
    a = a & b;
    return a;
}

inline NeuronSection operator| (const NeuronSection a, const NeuronSection b) noexcept
{
    return static_cast<NeuronSection>(static_cast<uint8_t>(a)
                                      | static_cast<uint8_t>(b));
}

inline NeuronSection& operator|= (NeuronSection& a, const NeuronSection& b) noexcept
{
    a = a | b;
    return a;
}

template<>
inline std::vector<std::string> EnumWrapper<NeuronSection>::toStringList() const noexcept
{
    return {SOMA_NAME, AXON_NAME, DENDRITE_NAME, APICALDENDRITE_NAME};
}

template<>
inline EnumWrapper<NeuronSection>::Types
EnumWrapper<NeuronSection>::fromString(const std::string& src) const
{
    const auto srcLc = brayns::string_utils::toLowercase(src);
    if(src == SOMA_NAME)
        return NeuronSection::SOMA;
    else if(src == AXON_NAME)
        return NeuronSection::AXON;
    else if(src == DENDRITE_NAME)
        return NeuronSection::DENDRITE;
    else if(src == APICALDENDRITE_NAME)
        return NeuronSection::APICAL_DENDRITE;

    throw std::invalid_argument("NeuronSection: Unknown section name '" + src + "'");
}

template<>
inline std::string
EnumWrapper<NeuronSection>::toString(const EnumWrapper<NeuronSection>::Types type) const noexcept
{
    std::vector<std::string> tokens;
    if(static_cast<uint8_t>(type & NeuronSection::SOMA))
        tokens.emplace_back(SOMA_NAME);
    if(static_cast<uint8_t>(type & NeuronSection::AXON))
        tokens.emplace_back(AXON_NAME);
    if(static_cast<uint8_t>(type & NeuronSection::DENDRITE))
        tokens.emplace_back(DENDRITE_NAME);
    if(static_cast<uint8_t>(type & NeuronSection::APICAL_DENDRITE))
        tokens.emplace_back(APICALDENDRITE_NAME);

    return brayns::string_utils::join(tokens, ",");
}

