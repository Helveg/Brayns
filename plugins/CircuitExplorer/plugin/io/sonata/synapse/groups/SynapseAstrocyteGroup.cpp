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

#include "SynapseAstrocyteGroup.h"

void SynapseAstrocyteGroup::addSynapse(const uint64_t id,
                                       const int32_t section,
                                       const float distance)
{
    _ids.push_back(id);
    _sections.push_back(section);
    _distances.push_back(distance);
}

void SynapseAstrocyteGroup::mapToCell(const MorphologyInstance& cell)
{
    for(size_t i = 0; i < _ids.size(); ++i)
    {
        const auto section = _sections[i];
        const auto segmentCount = cell.getSectionSegmentCount(section);
        if(segmentCount == 0)
            continue;

        const auto distance = _distances[i];

        float totalDistance = 0.f;
        std::vector<float> localDistances (segmentCount, 0.f);
        for(size_t i = 0; i < segmentCount; ++i)
        {
            const auto points = cell.getSegment(section, i);
            const auto dist = glm::length(*points.first - *points.second);
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
                const auto points = cell.getSegment(section, i);
                const auto point = glm::lerp(*points.first, *points.second, distance / localNorm);
                _geometry.push_back(brayns::createSDFSphere(point, 2.f));
                // By default we copy the cell simulation mapping to show the node
                // simulation report. If there is a synapse report, it will overwrite
                // this value
                _geometry.back().userData = cell.getSegmentSimulationOffset(section, i);
                break;
            }
        }

        _addedSynapses.push_back(i);
    }
}

void SynapseAstrocyteGroup::mapSimulation(const std::unordered_map<uint64_t, uint64_t>& mapping)
{
    for(size_t i = 0; i < _addedSynapses.size(); ++i)
    {
        auto it = mapping.find(_addedSynapses[i]);
        if(it != mapping.end())
            _geometry[i].userData = it->second;
    }
}

void SynapseAstrocyteGroup::addToModel(brayns::Model& model) const
{
    for(size_t i = 0; i < _addedSynapses.size(); ++i)
    {
        const auto matId = model.getMaterials().size();
        model.createMaterial(matId, "");
        model.addSDFGeometry(matId, _geometry[i], {});
    }
}
