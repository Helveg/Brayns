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
#include <brayns/common/types.h>

#include <plugin/api/CircuitColorHandler.h>


class MorphologySynapses
{
public:
    virtual ~MorphologySynapses() = default;
};

/**
 * @brief The MorphologyInstance class is the base class to implement representations of
 *        a cell geometry, and provides functionality to add simulation mapping
 *        based on the cell data.
 */
class MorphologyInstance
{
public:
    virtual ~MorphologyInstance() = default;

    virtual void mapSimulation(const size_t globalOffset,
                               const std::vector<uint16_t>& sectionOffsets,
                               const std::vector<uint16_t>& sectionCompartments) = 0;

    virtual void addToModel(brayns::Model& model) const = 0;

    virtual size_t
    getSectionSegmentCount(const int32_t section) const = 0;

    virtual std::pair<const brayns::Vector3f*, const brayns::Vector3f*>
    getSegment(const int32_t section, const uint32_t segment) const = 0;

    virtual uint64_t
    getSegmentSimulationOffset(const int32_t section, const uint32_t segment) const = 0;
};

using MorphologyInstancePtr = std::unique_ptr<MorphologyInstance>;
