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

namespace
{
constexpr char VEIN_NAME[]                  = "vein";
constexpr char ARTERY_NAME[]                = "artery";
constexpr char VENULE_NAME[]                = "venule";
constexpr char ARTERIOLE_NAME[]             = "arteriole";
constexpr char VENOUS_CAPILLARY_NAME[]      = "venous capillary";
constexpr char ARTERIAL_CAPILLARY_NAME[]    = "arterial capillary";
constexpr char TRANSITIONAL_NAME[]          = "transitional";
}

enum class VasculatureSection : uint8_t
{
    NONE                = 0,
    VEIN                = 1,
    ARTERY              = 2,
    VENULE              = 4,
    ARTERIOLE           = 8,
    VENOUS_CAPILLARY    = 16,
    ARTERIAL_CAPILLARY  = 32,
    TRANSITIONAL        = 64,
    ALL = VEIN | ARTERY | VENULE | ARTERIOLE | VENOUS_CAPILLARY | TRANSITIONAL
};

inline VasculatureSection operator& (const VasculatureSection a,
                                     const VasculatureSection b) noexcept
{
    return static_cast<VasculatureSection>(static_cast<uint8_t>(a)
                                           &static_cast<uint8_t>(b));
}

inline VasculatureSection operator&= (const VasculatureSection a,
                                      const VasculatureSection b) noexcept
{
    return a & b;
}

inline VasculatureSection operator| (const VasculatureSection a,
                                     const VasculatureSection b) noexcept
{
    return static_cast<VasculatureSection>(static_cast<uint8_t>(a)
                                           | static_cast<uint8_t>(b));
}

inline VasculatureSection operator|= (const VasculatureSection a,
                                      const VasculatureSection b) noexcept
{
    return a | b;
}

template<>
inline std::vector<std::string> EnumWrapper<VasculatureSection>::toStringList() const noexcept
{
    return {VEIN_NAME, ARTERY_NAME, VENULE_NAME, ARTERIOLE_NAME,
            VENOUS_CAPILLARY_NAME, ARTERIAL_CAPILLARY_NAME, TRANSITIONAL_NAME};
}

template<>
inline EnumWrapper<VasculatureSection>::Types
EnumWrapper<VasculatureSection>::fromString(const std::string& src) const
{
    const auto srcLc = brayns::string_utils::toLowercase(src);
    if(src == VEIN_NAME)
        return Types::VEIN;
    else if(src == ARTERY_NAME)
        return Types::ARTERY;
    else if(src == VENULE_NAME)
        return Types::VENULE;
    else if(src == ARTERIOLE_NAME)
        return Types::ARTERIOLE;
    else if(src == VENOUS_CAPILLARY_NAME)
        return Types::VENOUS_CAPILLARY;
    else if(src == ARTERIAL_CAPILLARY_NAME)
        return Types::ARTERIAL_CAPILLARY;
    else if(src == TRANSITIONAL_NAME)
        return Types::TRANSITIONAL;

    throw std::invalid_argument("VasculatureSection: Unknown section name '" + src + "'");
}

template<>
inline std::string
EnumWrapper<VasculatureSection>::toString(
        const EnumWrapper<VasculatureSection>::Types type) const noexcept
{
    std::vector<std::string> tokens;
    if(static_cast<uint8_t>(type & Types::VEIN))
        tokens.emplace_back(VEIN_NAME);
    if(static_cast<uint8_t>(type & Types::ARTERY))
        tokens.emplace_back(ARTERY_NAME);
    if(static_cast<uint8_t>(type & Types::VENULE))
        tokens.emplace_back(VENULE_NAME);
    if(static_cast<uint8_t>(type & Types::ARTERIOLE))
        tokens.emplace_back(ARTERIOLE_NAME);
    if(static_cast<uint8_t>(type & Types::VENOUS_CAPILLARY))
        tokens.emplace_back(VENOUS_CAPILLARY_NAME);
    if(static_cast<uint8_t>(type & Types::ARTERIAL_CAPILLARY))
        tokens.emplace_back(ARTERIAL_CAPILLARY_NAME);
    if(static_cast<uint8_t>(type & Types::TRANSITIONAL))
        tokens.emplace_back(TRANSITIONAL_NAME);

    return brayns::string_utils::join(tokens, ",");
}
