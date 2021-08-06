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

#include <brayns/common/geometry/Cone.h>
#include <brayns/common/geometry/Cylinder.h>
#include <brayns/common/geometry/Sphere.h>

#include <unordered_map>

/**
 * @brief The PrimitiveGeometryBuilder class is a builder that transform a Morphology
 *        object into primitive shapes (sheres, cones and cylinders)
 */
class PrimitiveGeometryBuilder : public MorphologyGeometryBuilder
{
public:
    void build(const Morphology&) final;
    std::unique_ptr<MorphologyInstance> instantiate(const brayns::Vector3f&,
                                                    const brayns::Quaternion&) const final;

private:
    struct Geometry
    {
        std::vector<brayns::Sphere> spheres;
        std::vector<brayns::Cylinder> cylinders;
        std::vector<brayns::Cone> cones;
    };
    // Holds the geometry
    std::vector<Geometry> geometry;
    // Maps each section type (soma, axon, dendrite and apical dendrite)
    // to the geometries of that type
    std::unordered_map<MorphologySection, std::vector<size_t>> sectionTypeGeometryMap;
    // Maps each section with its geometry
    std::unordered_map<uint32_t, size_t> sectionGeometryMap;
};
