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

#include <plugin/io/morphology/neuron/NeuronBuilder.h>
#include <plugin/io/morphology/neuron/instances/PrimitiveNeuronInstance.h>

#include <brayns/common/geometry/Cone.h>
#include <brayns/common/geometry/Cylinder.h>
#include <brayns/common/geometry/Sphere.h>

#include <unordered_map>

/**
 * @brief The PrimitiveGeometryBuilder class is a builder that transform a Morphology
 *        object into primitive shapes (sheres, cones and cylinders)
 */

using SectionGeometry = std::unordered_map<int32_t, std::vector<size_t>>;

class PrimitiveNeuronBuilder: public NeuronBuilder
{
public:
    void _buildImpl(const NeuronMorphology&) final;
    MorphologyInstancePtr _instantiateImpl(const brayns::Vector3f&,
                                           const brayns::Quaternion&) const final;
private:
    std::vector<brayns::Sphere> _spheres;
    std::vector<brayns::Cylinder> _cylinders;
    std::vector<brayns::Cone> _cones;

    std::shared_ptr<PrimitiveSharedData> _data;

    void addSphere(const brayns::Vector3f& c,
                   const float r,
                   const NeuronSection section,
                   const int32_t sectionId) noexcept;
    void addCylinder(const brayns::Vector3f& c,
                     const brayns::Vector3f& u,
                     const float r,
                     const NeuronSection section,
                     const int32_t sectionId) noexcept;
    void addCone(const brayns::Vector3f& c,
                 const float r,
                 const brayns::Vector3f& u,
                 const float ru,
                 const NeuronSection section,
                 const int32_t sectionId) noexcept;
};
