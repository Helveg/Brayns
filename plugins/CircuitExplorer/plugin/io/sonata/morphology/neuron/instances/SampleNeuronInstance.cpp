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

#include "SampleNeuronInstance.h"

#include <brayns/engine/Material.h>
#include <brayns/engine/Model.h>

namespace
{
inline auto createMatrial(brayns::Model& model)
{
    const auto newMatId = model.getMaterials().size();
    model.createMaterial(newMatId, "");
    return newMatId;
}
}

SampleNeuronInstance::SampleNeuronInstance(std::vector<brayns::Sphere>&& geometry,
                                           const std::shared_ptr<SampleSharedData>& data)
 : _samples(std::move(geometry))
 , _data(data)
{
}

/*
void SampleMorphologyInstance::addSynapses(const CellEdgeData& data)
{
    const CellSynapseData* synapses = dynamic_cast<const CellSynapseData*>(&data);
    if(synapses != nullptr)
    {
        for(size_t i = 0; i < synapses->edgeIds.size(); ++i)
        {
            const auto synapseId = synapses->edgeIds[i];
            const auto sectionId = synapses->sectionIds[i];
            const auto& surfacePosition = synapses->surfacePos[i];

            auto it = _sectionGeometry.find(sectionId);
            // The synapse might belong to a section we havent loaded
            if(it == _sectionGeometry.end())
                return;

            // TEMPORARY FIX BECAUSE THE EXAMPLE USES CASES EDGE FILES LACK [afferent|efferent]_section_pos
            size_t closest = 0;
            for(size_t i = 0; i < it->second.size() - 1; ++i)
            {
                const auto& segment = it->second[i];
                const auto& p0 = _samples[segment.startGeomId].center;
                const auto& p1 = _samples[segment.endGeomId].center;
                const auto d = glm::dot(surfacePosition - p0, surfacePosition - p1);
                if(d < 0.f)
                {
                    closest = i;
                    break;
                }
            }

            const auto synapseGeomId = _samples.size();
            _samples.push_back(brayns::Sphere(surfacePosition, 0.4f));
            it->second[closest].synapseGeomIds.push_back({synapseId, synapseGeomId});
        }
        return;
    }

    const AstrocyteSynapseData* astroSynapses = dynamic_cast<const AstrocyteSynapseData*>(&data);
    if(astroSynapses)
    {
        for(size_t i = 0; i < astroSynapses->edgeIds.size(); ++i)
        {
            const auto synapseId = astroSynapses->edgeIds[i];
            const auto sectionId = astroSynapses->sectionIds[i];
            const auto distance = astroSynapses->distances[i];

            auto it = _sectionGeometry.find(sectionId);
            // The synapse might belong to a section we havent loaded
            if(it == _sectionGeometry.end())
                return;

            float totalDistance = 0.f;
            std::vector<float> localDistances (it->second.size(), 0.f);
            for(size_t i = 0; i < it->second.size(); ++i)
            {
                const auto& segment = it->second[i];
                const auto& start = _samples[segment.startGeomId];
                const auto& end = _samples[segment.endGeomId];
                const auto dist = glm::length(start.center - end.center);
                totalDistance += dist;
                localDistances[i] = dist;
            }
            const float invTotalDist = 1.f / totalDistance;

            float traversedDistance = 0.f;
            for(size_t i = 0; i < localDistances.size(); ++i)
            {
                traversedDistance += localDistances[i];
                const float localNorm = traversedDistance * invTotalDist;
                if(localNorm >= distance)
                {
                    const auto& segment = it->second[i];
                    const auto& start = _samples[segment.startGeomId];
                    const auto& end = _samples[segment.endGeomId];
                    const auto point = glm::lerp(start.center, end.center, distance / localNorm);
                    const auto synapseGeomId = _samples.size();
                    _samples.emplace_back(point, 2.f, 0);
                    it->second[i].synapseGeomIds.push_back({synapseId, synapseGeomId});
                    break;
                }
            }
        }
        return;
    }
}
*/

void SampleNeuronInstance::mapSimulation(const size_t globalOffset,
                                         const std::vector<uint16_t>& sectionOffsets,
                                         const std::vector<uint16_t>& sectionCompartments)
{
    if(sectionOffsets.empty())
        return;

    for(auto& geomSection : _data->sectionMap)
    {
        const auto& segments = geomSection.second;
        // No section level information (soma report, spike simulation, etc.)
        // or dealing with soma
        if(geomSection.first == -1 || sectionOffsets.empty()
                || geomSection.first > sectionOffsets.size() - 1)
        {
            for(const auto& segment : segments)
                _samples[segment].userData = globalOffset;
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
                _samples[segments[i]].userData = finalOffset;
            }
        }
    }
}

void SampleNeuronInstance::addToModel(brayns::Model& model) const
{
    // Add geometries to the model
    std::unordered_map<NeuronSection, size_t> sectionToMat;
    for (size_t i = 0; i < _samples.size(); ++i)
    {
        const auto sectionType = _data->sectionTypes[i];
        auto it = sectionToMat.find(sectionType);
        size_t materialId = 0;
        if(it == sectionToMat.end())
        {
            materialId = createMatrial(model);
            sectionToMat[sectionType] = materialId;
        }
        else
            materialId = it->second;

        model.addSphere(materialId, _samples[i]);
    }
}

size_t
SampleNeuronInstance::getSectionSegmentCount(const int32_t section) const
{
    auto it = _data->sectionMap.find(section);
    if(it == _data->sectionMap.end())
        throw std::invalid_argument("Section " + std::to_string(section) + "not found");

    return std::max(it->second.size(), 1ul) - 1;
}

std::pair<const brayns::Vector3f*, const brayns::Vector3f*>
SampleNeuronInstance::getSegment(const int32_t section, const uint32_t segment) const
{
    auto it = _data->sectionMap.find(section);
    if(it == _data->sectionMap.end())
        throw std::invalid_argument("Section " + std::to_string(section) + " not found");

    if(std::max(it->second.size(), 1ul) - 1 <= segment)
        throw std::invalid_argument("Section " + std::to_string(section) + " "
                                    "Segment " + std::to_string(segment) + " not found");

    const auto& start = _samples[it->second[segment]];
    const auto& end = _samples[it->second[segment + 1]];
    return std::make_pair(&start.center, &end.center);
}

uint64_t
SampleNeuronInstance::getSegmentSimulationOffset(const int32_t section, const uint32_t segment) const
{
    auto it = _data->sectionMap.find(section);
    if(it == _data->sectionMap.end())
        throw std::invalid_argument("Section " + std::to_string(section) + " not found");

    if(it->second.size() <= segment)
        throw std::invalid_argument("Section " + std::to_string(section) + " "
                                    "Segment " + std::to_string(segment) + " not found");

    return _samples[it->second[segment]].userData;
}
