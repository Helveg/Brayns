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

#include "VasculatureSDFInstance.h"

#include <brayns/engine/Model.h>

// There is not much difference between SDFNeuronInstance and VasculatureSDFInstance
// They are separated in anticipation that something must be done for one and not for another
// Currently, the only difference are the sectionTypes of the geometry, and the optimization of
// not having to copy the geometries since the vasculature is already stored in world space
// and does not need to be trasnformed.

namespace
{
inline auto createMaterial(brayns::Model& model)
{
    const auto newMatId = model.getMaterials().size();
    model.createMaterial(newMatId, "");
    return newMatId;
}
}

VasculatureSDFInstance::VasculatureSDFInstance(const std::shared_ptr<Geometry>& geometry)
 : _geometry(geometry)
{
}

void VasculatureSDFInstance::mapSimulation(const size_t globalOffset,
                                        const std::vector<uint16_t>& sectionOffsets,
                                        const std::vector<uint16_t>& sectionCompartments)
{
    for(auto& geomSection : _geometry->sections)
    {
        const auto& segments = geomSection.second;
        if(sectionOffsets.empty() || geomSection.first > sectionOffsets.size() - 1)
        {
            for(const auto& segment : segments)
                _geometry->objects[segment].userData = globalOffset;
        }
        else
        {
            const double step = static_cast<double>(sectionCompartments[geomSection.first])
                    / static_cast<double>(segments.size());

            const size_t sectionOffset = sectionOffsets[geomSection.first];
            for(size_t i = 0; i < segments.size(); ++i)
            {
                const auto compartment = static_cast<size_t>(std::floor(step * i));
                const auto finalOffset = globalOffset + (sectionOffset + compartment);
                _geometry->objects[segments[i]].userData = finalOffset;
            }
        }
    }
}

void VasculatureSDFInstance::addToModel(brayns::Model& model) const
{
    std::vector<size_t> localToGlobalIndex(_geometry->objects.size(), 0);

    // Add geometries to Model. We do not know the indices of the neighbours
    // yet so we leave them empty.
    std::unordered_map<VasculatureSection, size_t> sectionToMat;
    for (size_t i = 0; i < _geometry->sectionTypes.size(); ++i)
    {
        const auto sectionType =  _geometry->sectionTypes[i];
        auto it = sectionToMat.find(sectionType);
        size_t materialId = 0;
        if(it == sectionToMat.end())
        {
            materialId = createMaterial(model);
            sectionToMat[sectionType] = materialId;
        }
        else
            materialId = it->second;

        localToGlobalIndex[i] =
            model.addSDFGeometry(materialId, _geometry->objects[i], {});
    }

    // Write the neighbours using global indices
    std::vector<size_t> neighboursTmp;
    for (size_t i = 0; i < _geometry->sectionTypes.size(); ++i)
    {
        const size_t globalIndex = localToGlobalIndex[i];
        neighboursTmp.clear();

        for (auto localNeighbourIndex : _geometry->neighbours[i])
            neighboursTmp.push_back(
                localToGlobalIndex[localNeighbourIndex]);

        model.updateSDFGeometryNeighbours(globalIndex, neighboursTmp);
    }
}

size_t VasculatureSDFInstance::getSectionSegmentCount(const int32_t section) const
{
    auto it = _geometry->sections.find(section);
    if(it == _geometry->sections.end())
        throw std::invalid_argument("Section " + std::to_string(section) + " not found");

    return it->second.size();
}

std::pair<const brayns::Vector3f*, const brayns::Vector3f*>
VasculatureSDFInstance::getSegment(const int32_t section, const uint32_t segment) const
{
    auto it = _geometry->sections.find(section);
    if(it == _geometry->sections.end())
        throw std::invalid_argument("Section " + std::to_string(section) + " not found");

    if(it->second.size() <= segment)
        throw std::invalid_argument("Section " + std::to_string(section) + " "
                                    "Segment " + std::to_string(segment) + " not found");

    const auto& geom = _geometry->objects[it->second[segment]];
    return std::make_pair(&geom.p0, &geom.p1);
}

uint64_t VasculatureSDFInstance::getSegmentSimulationOffset(const int32_t section,
                                                         const uint32_t segment) const
{
    auto it = _geometry->sections.find(section);
    if(it == _geometry->sections.end())
        throw std::invalid_argument("Section " + std::to_string(section) + " not found");

    if(it->second.size() <= segment)
        throw std::invalid_argument("Section " + std::to_string(section) + " "
                                    "Segment " + std::to_string(segment) + " not found");

    return _geometry->objects[it->second[segment]].userData;
}
