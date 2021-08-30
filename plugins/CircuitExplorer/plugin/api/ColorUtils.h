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

#include <brayns/common/mathTypes.h>

struct ColorTable
{
    std::vector<brayns::Vector3f> VALUES;

    ColorTable();
};

class ColorDeck
{
public:
    const brayns::Vector3f& getColorForKey(const std::string& k) noexcept;

private:
    const brayns::Vector3f& emplaceColor(const std::string& k) noexcept;

    static ColorTable _TABLE;

    std::unordered_map<std::string, size_t> _colorMap;
    size_t _lastIndex {0};
};

class ColorRoulette
{
public:
    const brayns::Vector3f& getNextColor() noexcept;

private:
    static ColorTable _TABLE;

    size_t _lastIndex {0};
};
