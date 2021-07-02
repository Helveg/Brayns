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

namespace morphology
{
class SDFMorphologyInstance: public MorphologyInstance
{
public:
    SDFMorphologyInstance(const std::vector<brayns::SDFGeometry>& geometry,
                          const std::vector<std::vector<size_t>>& neighbours,
                          const std::vector<SectionType>& sectionTypeMap,
                          const std::unordered_map<int32_t, std::vector<size_t>>& sectionSegments);

    void transform(const brayns::Vector3f& translation,
                   const brayns::Quaternion& rotation) final;

    void addSynapse(const std::string& srcEdgePopulation,
                    const brayns::Vector3f& pos,
                    const uint64_t edgeId,
                    const int32_t section,
                    const bool isAfferent) final;

    void mapSimulation(const size_t globalOffset,
                       const std::vector<uint16_t>& sectionOffsets,
                       const std::vector<uint16_t>& sectionCompartments) final;

    CellGeometryMap addToModel(brayns::Model& model) const final;

private:
    std::vector<brayns::SDFGeometry> _sdfGeometries;
    std::vector<std::vector<size_t>> _sdfNeighbours;
    std::vector<SectionType> _sdfSectionTypes;

    struct SDFSynapse
    {
        bool afferent;
        uint64_t edgeId;
        size_t geomIdx;
    };

    size_t _synapseGeometryOffset;
    std::vector<SDFSynapse> _sdfSynapses;

    struct SDFSegment
    {
        const size_t geomIdx;
        // Maps edge population with all the synapse geometries that belongs to that population
        std::unordered_map<std::string, std::vector<size_t>> synGeomIndices;

        SDFSegment(const size_t geomIdx)
         : geomIdx(geomIdx)
        {
        }
    };

    // This structure holds only the map of sections to the geometry that represents it
    std::unordered_map<int32_t, std::vector<SDFSegment>> _sectionMap;
};
}
