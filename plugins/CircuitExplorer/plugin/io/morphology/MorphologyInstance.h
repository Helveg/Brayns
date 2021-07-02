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

namespace morphology
{
class MorphologyInstance
{
public:
    virtual ~MorphologyInstance() = default;

    virtual void transform(const brayns::Vector3f& translation,
                           const brayns::Quaternion& rotation) = 0;
    virtual void addSynapse(const std::string& srcEdgePopulation,
                            const brayns::Vector3f& pos,
                            const uint64_t edgeId,
                            const int32_t section,
                            const bool isAfferent) = 0;
    virtual void mapSimulation(const size_t globalOffset,
                               const std::vector<uint16_t>& sectionOffsets,
                               const std::vector<uint16_t>& sectionCompartments) = 0;
    virtual CellGeometryMap addToModel(brayns::Model& model) const = 0;
};
}
