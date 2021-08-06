#include "SDFMorphologyInstance.h"

#include <brayns/engine/Material.h>
#include <brayns/engine/Model.h>

#include <iostream>

namespace
{
inline auto createMatrial(brayns::Model& model, const brayns::Vector3d& color)
{
    const auto newMatId = model.getMaterials().size();
    auto mat = model.createMaterial(newMatId, "");
    mat->setDiffuseColor(color);
    mat->markModified();
    return newMatId;
}
}

SDFMorphologyInstance::SDFMorphologyInstance(
        std::vector<brayns::SDFGeometry> geometry,
        std::vector<std::vector<size_t>> neighbours,
        std::vector<MorphologySection> sectionTypeMap,
        SectionGeometry sectionSegments)
 : _sdfGeometries(std::move(geometry))
 , _sdfNeighbours(std::move(neighbours))
 , _sdfSectionTypes(std::move(sectionTypeMap))
{
    for(const auto& entry : sectionSegments)
    {
        auto& segments = _sectionGeometry[entry.first];
        segments.reserve(entry.second.size());
        for(const auto& segmentId : entry.second)
            segments.push_back({segmentId, {}});
    }
}

void SDFMorphologyInstance::addSynapse(const uint64_t synapseId,
                                       const int32_t sectionId,
                                       const float sectionDistance,
                                       const brayns::Vector3f& surfacePosition)
{
    auto it = _sectionGeometry.find(sectionId);
    // The synapse might belong to a section we havent loaded
    if(it == _sectionGeometry.end())
        return;

    // TEMPORARY FIX BECAUSE THE EXAMPLE USES CASES EDGE FILES LACK [afferent|efferent]_section_pos
    size_t closest = 0;
    for(size_t i = 0; i < it->second.size(); ++i)
    {
        const auto& segment = it->second[i];
        const auto& sdfGeom = _sdfGeometries[segment.geomId];
        const auto d = glm::dot(surfacePosition - sdfGeom.p0, surfacePosition - sdfGeom.p1);
        if(d < 0.f)
        {
            closest = i;
            break;
        }
    }

    const auto& sdfGeom = _sdfGeometries[it->second[closest].geomId];
    const auto dirVector = glm::normalize(sdfGeom.p1 - sdfGeom.p0);
    const auto projectedLen = glm::dot(dirVector, surfacePosition - sdfGeom.p0);
    const auto point = sdfGeom.p0 + dirVector * projectedLen;
    const auto synDirVector = glm::normalize(surfacePosition - point);
    const auto endPoint = point + synDirVector * 3.0f;
    const auto synapseGeomId = _sdfGeometries.size();
    _sdfGeometries.push_back(brayns::createSDFConePillSigmoid(endPoint,
                                                              surfacePosition,
                                                              0.35f,
                                                              0.25f));
    it->second[closest].synapseGeomIds.push_back({synapseId, synapseGeomId});

    /*
    float totalDistance = 0.f;
    std::vector<float> localDistances (it->second.size(), 0.f);
    for(size_t i = 0; i < it->second.size(); ++i)
    {
        const auto& segment = it->second[i];
        const auto& sdfGeom = _sdfGeometries[segment.geomId];
        const auto dist = glm::length(sdfGeom.p1 - sdfGeom.p0);
        totalDistance += dist;
        localDistances[i] = dist;
    }
    const float invTotalDist = 1.f / totalDistance;

    float traversedDistance = 0.f;
    for(size_t i = 0; i < localDistances.size(); ++i)
    {
        traversedDistance += localDistances[i];
        const float localNorm = traversedDistance * invTotalDist;
        if(localNorm >= sectionDistance)
        {
            const auto& segment = it->second[i];
            const auto point = glm::lerp(_sdfGeometries[segment.geomId].p0,
                                         _sdfGeometries[segment.geomId].p1,
                                         sectionDistance / localNorm);
            const auto synapseGeomId = _sdfGeometries.size();
            _sdfGeometries.push_back(brayns::createSDFConePillSigmoid(point,
                                                                      surfacePosition,
                                                                      0.25f,
                                                                      0.35f));
            it->second[i].synapseGeomIds.push_back({synapseId, synapseGeomId});
            break;
        }
    }
    */
}

void SDFMorphologyInstance::mapSimulation(const size_t globalOffset,
                                          const std::vector<uint16_t>& sectionOffsets,
                                          const std::vector<uint16_t>& sectionCompartments)
{
    if(sectionOffsets.empty())
        return;

    for(auto& geomSection : _sectionGeometry)
    {
        const auto& segments = geomSection.second;
        // No section level information (soma report, spike simulation, etc.)
        // or dealing with soma
        if(geomSection.first == -1 || geomSection.first > sectionOffsets.size() - 1)
        {
            for(const auto& segment : segments)
            {
                _sdfGeometries[segment.geomId].userData = globalOffset;
                for(const auto& synapse : segment.synapseGeomIds)
                    _sdfGeometries[synapse.geomId].userData = globalOffset;
            }
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
                _sdfGeometries[segments[i].geomId].userData = finalOffset;
                for(const auto& synapse : segments[i].synapseGeomIds)
                    _sdfGeometries[synapse.geomId].userData = finalOffset;
            }
        }
    }
}

CellGeometryMap SDFMorphologyInstance::addToModel(brayns::Model& model) const
{

    std::vector<size_t> localToGlobalIndex(_sdfSectionTypes.size(), 0);

    // Add geometries to Model. We do not know the indices of the neighbours
    // yet so we leave them empty.
    std::unordered_map<MorphologySection, size_t> sectionToMat;
    for (size_t i = 0; i < _sdfSectionTypes.size(); ++i)
    {
        const auto sectionType = _sdfSectionTypes[i];
        auto it = sectionToMat.find(sectionType);
        size_t materialId = 0;
        if(it == sectionToMat.end())
        {
            materialId = createMatrial(model, {1., 0., 0.});
            sectionToMat[sectionType] = materialId;
        }
        else
            materialId = it->second;

        localToGlobalIndex[i] =
            model.addSDFGeometry(materialId, _sdfGeometries[i], {});
    }
    // Write the neighbours using global indices
    std::vector<size_t> neighboursTmp;
    for (size_t i = 0; i < _sdfSectionTypes.size(); ++i)
    {
        const size_t globalIndex = localToGlobalIndex[i];
        neighboursTmp.clear();

        for (auto localNeighbourIndex : _sdfNeighbours[i])
            neighboursTmp.push_back(
                localToGlobalIndex[localNeighbourIndex]);

        model.updateSDFGeometryNeighbours(globalIndex, neighboursTmp);
    }

    // Add the syanapses
    for(const auto& section : _sectionGeometry)
    {
        for(const auto& segment : section.second)
        {
            for(const auto& synapse : segment.synapseGeomIds)
            {
                const auto matId = createMatrial(model, {0., 1., 0.});
                model.addSDFGeometry(matId, _sdfGeometries[synapse.geomId], {});
            }
        }
    }

    return {};
}
