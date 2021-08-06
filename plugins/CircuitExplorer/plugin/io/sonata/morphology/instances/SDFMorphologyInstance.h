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

#include "../Morphology.h"
#include "../MorphologyInstance.h"

#include <brayns/common/geometry/SDFGeometry.h>

#include <unordered_map>


/**
 * @brief The PrimitiveMorphologyInstance class represents a cell 3D shape based
 *        on SDF Geometry
 */

using SectionGeometry = std::unordered_map<int32_t, std::vector<size_t>>;

class SDFMorphologyInstance: public MorphologyInstance
{
public:
    SDFMorphologyInstance(std::vector<brayns::SDFGeometry> geometry,
                          std::vector<std::vector<size_t>> neighbours,
                          std::vector<MorphologySection> sectionTypeMap,
                          SectionGeometry sectionSegments);

    void addSynapse(const uint64_t synapseId,
                    const int32_t sectionId,
                    const float sectionDistance,
                    const brayns::Vector3f& surfacePosition) final;

    void mapSimulation(const size_t globalOffset,
                       const std::vector<uint16_t>& sectionOffsets,
                       const std::vector<uint16_t>& sectionCompartments) final;

    CellGeometryMap addToModel(brayns::Model& model) const final;

private:
    std::vector<brayns::SDFGeometry> _sdfGeometries;
    std::vector<std::vector<size_t>> _sdfNeighbours;
    std::vector<MorphologySection> _sdfSectionTypes;

    struct Synapse
    {
        uint64_t synapseId;
        size_t geomId;
    };

    struct Segment
    {
        size_t geomId;
        std::vector<Synapse> synapseGeomIds;
    };

    // This structure holds only the map of sections to the segment geometry that represents it
    std::unordered_map<int32_t, std::vector<Segment>> _sectionGeometry;
};
