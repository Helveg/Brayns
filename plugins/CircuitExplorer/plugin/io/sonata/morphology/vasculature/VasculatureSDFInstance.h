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
#include <plugin/io/sonata/morphology/MorphologyInstance.h>

#include <brayns/common/geometry/SDFGeometry.h>

#include <unordered_map>

/**
 * @brief The VasculatureSDFInstance class represents a vasculature geometry to
 *        be placed on the scene and to which simulation can be mapped. It also
 *        gives access to geometry information at the section/segment level
 *        to allow for external geometry surface mapping (unused for vasculature)
 */
class VasculatureSDFInstance : public MorphologyInstance
{
public:
    struct Geometry
    {
        std::vector<brayns::SDFGeometry> objects;
        std::vector<std::vector<size_t>> neighbours;
        std::vector<VasculatureSection> sectionTypes;
        std::unordered_map<int32_t, std::vector<size_t>> sections;
    };

    VasculatureSDFInstance(const std::shared_ptr<Geometry>& geometry);

    void mapSimulation(const size_t globalOffset,
                       const std::vector<uint16_t>& sectionOffsets,
                       const std::vector<uint16_t>& sectionCompartments) final;

    void addToModel(brayns::Model& model) const final;

    size_t getSectionSegmentCount(const int32_t section) const final;

    std::pair<const brayns::Vector3f*, const brayns::Vector3f*>
    getSegment(const int32_t section, const uint32_t segment) const final;

    uint64_t getSegmentSimulationOffset(const int32_t section, const uint32_t segment) const final;

private:
    std::shared_ptr<Geometry> _geometry;
};
