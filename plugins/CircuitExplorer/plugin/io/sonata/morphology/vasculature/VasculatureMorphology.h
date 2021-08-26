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

#include <plugin/io/sonata/SonataLoaderEnums.h>

#include <brayns/common/mathTypes.h>

#include <string>
#include <unordered_set>
#include <vector>

/**
 * @brief The VasculatureMorphology class represents a vasculature dataset as a list of
 *        sections, each section being composed of list of segments. Each segment consists
 *        on a start point and radius and an end point an radius. The coordinates are in
 *        world space.
 */
class VasculatureMorphology
{
public:
    struct Segment
    {
        brayns::Vector3f start;
        float startRadius;
        brayns::Vector3f end;
        float endRadius;
    };

    struct Section
    {
        uint32_t id;
        uint32_t parentId;
        VasculatureSection type;
        std::vector<Segment> segments;
    };

    std::vector<Section>& sections() noexcept
    {
        return _sections;
    }

    const std::vector<Section>& sections() const noexcept
    {
        return _sections;
    }

private:
    std::vector<Section> _sections;
};
