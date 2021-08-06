/* Copyright (c) 2015-2021, EPFL/Blue Brain Project
 * All rights reserved. Do not distribute without permission.
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

#include <brayns/common/mathTypes.h>
#include <brayns/common/types.h>

#include <plugin/api/CellMapper.h>

/**
 * @brief The MorphologyInstance class is the base class to implement representations of
 *        a cell geometry, and provides functionality to add simulation mapping
 *        based on the cell data.
 */
class MorphologyInstance
{
public:
    virtual ~MorphologyInstance() = default;

    virtual void addSynapse(const uint64_t synapseId,
                            const int32_t sectionId,
                            const float sectionDistance,
                            const brayns::Vector3f& surfacePosition) = 0;

    virtual void mapSimulation(const size_t globalOffset,
                               const std::vector<uint16_t>& sectionOffsets,
                               const std::vector<uint16_t>& sectionCompartments) = 0;

    virtual CellGeometryMap addToModel(brayns::Model& model) const = 0;
};

using MorphologyInstancePtr = std::unique_ptr<MorphologyInstance>;
