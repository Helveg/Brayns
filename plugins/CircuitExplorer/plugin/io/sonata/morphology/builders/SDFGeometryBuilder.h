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

#include "../MorphologyGeometryBuilder.h"

#include <unordered_map>


/**
 * @brief The PrimitiveGeometryBuilder class is a builder that transform a Morphology
 *        object into SDF (Signed distance field) geometry.
 */

class SDFGeometryBuilder : public MorphologyGeometryBuilder
{
public:
    void build(const Morphology&) final;
    std::unique_ptr<MorphologyInstance> instantiate(const brayns::Vector3f&,
                                                    const brayns::Quaternion&) const final;
private:
    class InternalBuilder;
    friend class InternalBuilder;

    brayns::Vector3f _somaPos;

    std::vector<brayns::SDFGeometry> _sdfGeometries;
    std::vector<std::vector<size_t>> _sdfNeighbours;
    std::vector<MorphologySection> _sdfSectionTypes;
    std::unordered_map<int32_t, std::vector<size_t>> _sectionMap;
};
