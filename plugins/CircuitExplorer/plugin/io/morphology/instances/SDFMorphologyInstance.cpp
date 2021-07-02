#include "SDFMorphologyInstance.h"

#include <brayns/engine/Model.h>

#include <iostream>

#define SYNAPSE_POS_DISPLACEMENT_MULTIPLIER 1.f

namespace morphology
{

namespace
{
inline auto createMatrial(brayns::Model& model)
{
    const auto newMatId = model.getMaterials().size();
    model.createMaterial(newMatId, "");
    return newMatId;
}
}

SDFMorphologyInstance::SDFMorphologyInstance(
        const std::vector<brayns::SDFGeometry>& geometry,
        const std::vector<std::vector<size_t>>& neighbours,
        const std::vector<SectionType>& sectionTypeMap,
        const std::unordered_map<int32_t, std::vector<size_t>>& sectionSegments)
 : _sdfGeometries(geometry)
 , _sdfNeighbours(neighbours)
 , _sdfSectionTypes(sectionTypeMap)
 , _synapseGeometryOffset(_sdfGeometries.size())
{
    for(const auto& entry : sectionSegments)
    {
        std::vector<SDFSegment>& segments = _sectionMap[entry.first];
        for(const auto& segment : entry.second)
            segments.emplace_back(segment);
    }
}

void SDFMorphologyInstance::transform(const brayns::Vector3f& translation,
                                      const brayns::Quaternion& rotation)
{
    for(auto& geometry : _sdfGeometries)
    {
        geometry.p0 = translation + rotation * geometry.p0;
        geometry.p1 = translation + rotation * geometry.p1;
    }
}

void SDFMorphologyInstance::addSynapse(const std::string& srcEdgePopulation,
                                       const brayns::Vector3f& pos,
                                       const uint64_t edgeId,
                                       const int32_t section,
                                       const bool isAfferent)
{
    auto sectionIt = _sectionMap.find(section);
    if(sectionIt == _sectionMap.end())
        return;

    // Choose the closest segment of the section
    auto& segments = sectionIt->second;
    size_t selectedSegment = 0;
    float closest = std::numeric_limits<float>::max();
    for(size_t i = 0; i < segments.size(); ++i)
    {
        const auto& geometry = _sdfGeometries[segments[i].geomIdx];
        const auto geomDirV = glm::normalize(geometry.p0 - geometry.p1);
        const auto synapseDirV = pos - geometry.p1;
        const auto projLen = std::abs(glm::dot(synapseDirV, geomDirV));
        if(projLen < closest)
        {
            closest = projLen;
            selectedSegment = i;
        }
    }

    auto& segmentObj = segments[selectedSegment];
    const auto& segmentGeom = _sdfGeometries[segmentObj.geomIdx];
    const auto& p0 = segmentGeom.p0;
    const auto& p1 = segmentGeom.p1;

    // Compute the 3D point on which the synapse will be born from the neurite
    const auto dirVector = glm::normalize(p1 - p0);
    const auto lenOnSegment = glm::clamp(glm::dot(glm::normalize(pos - p0), dirVector), 0.f, 1.f);
    const auto lerpRadius = glm::lerp(segmentGeom.r0, segmentGeom.r1, lenOnSegment);

    const auto pointOnSegment = glm::lerp(p0, p1, lenOnSegment);

    // Add the new geometry
    const auto newGeomIdx = _sdfGeometries.size();
    _sdfGeometries.push_back(brayns::createSDFConePillSigmoid(pointOnSegment,
                                                              pos,
                                                              lerpRadius * 1.35,
                                                              lerpRadius * 1.7));
    _sdfNeighbours.emplace_back();
    _sdfNeighbours[newGeomIdx].push_back(segmentObj.geomIdx);
    _sdfNeighbours[segmentObj.geomIdx].push_back(newGeomIdx);
    _sdfSynapses.push_back({isAfferent, edgeId, newGeomIdx});

    // Add the geometry reference to this segment so that we apply the same simulation offset
    // to the synapses
    segmentObj.synGeomIndices[srcEdgePopulation].push_back(newGeomIdx);
}

void SDFMorphologyInstance::mapSimulation(const size_t globalOffset,
                                          const std::vector<uint16_t>& sectionOffsets,
                                          const std::vector<uint16_t>& sectionCompartments)
{
    if(sectionOffsets.empty())
        return;

    for(auto& geomSection : _sectionMap)
    {
        const auto& segments = geomSection.second;
        // No section level information (soma report, spike simulation, etc.)
        // or dealing with soma
        if(geomSection.first == -1 || geomSection.first > sectionOffsets.size())
        {
            for(const auto& segment : segments)
                _sdfGeometries[segment.geomIdx].userData = globalOffset;
        }
        else
        {
            const double step = static_cast<double>(sectionCompartments[geomSection.first])
                    / static_cast<double>(segments.size());

            const size_t sectionOffset = sectionOffsets[geomSection.first];
            for(size_t i = 0; i < segments.size(); ++i)
            {
                const auto compartment = static_cast<size_t>(std::floor(step * i));
                _sdfGeometries[segments[i].geomIdx].userData =
                        globalOffset + (sectionOffset + compartment);
            }
        }
    }
}

CellGeometryMap SDFMorphologyInstance::addToModel(brayns::Model& model) const
{

    const size_t numGeoms = _sdfGeometries.size();
    std::vector<size_t> localToGlobalIndex(numGeoms, 0);

    // Add geometries to Model. We do not know the indices of the neighbours
    // yet so we leave them empty.
    // First we handle the cell itself
    std::unordered_map<SectionType, size_t> sectionToMat;
    for (size_t i = 0; i < _sdfSectionTypes.size(); i++)
    {
        const auto sectionType = _sdfSectionTypes[i];
        auto it = sectionToMat.find(sectionType);
        size_t materialId = 0;
        if(it == sectionToMat.end())
        {
            materialId = createMatrial(model);
            sectionToMat[sectionType] = materialId;
        }
        else
            materialId = it->second;

        localToGlobalIndex[i] =
            model.addSDFGeometry(materialId, _sdfGeometries[i], {});
    }

    // Then we handle the synapsees
    for(size_t i = _synapseGeometryOffset; i < _sdfGeometries.size(); ++i)
    {
        const auto synapseMaterial = createMatrial(model);
        localToGlobalIndex[i] =
                model.addSDFGeometry(synapseMaterial, _sdfGeometries[i], {});
    }

    // Write the neighbours using global indices
    std::vector<size_t> neighboursTmp;
    for (size_t i = 0; i < numGeoms; i++)
    {
        const size_t globalIndex = localToGlobalIndex[i];
        neighboursTmp.clear();

        for (auto localNeighbourIndex : _sdfNeighbours[i])
            neighboursTmp.push_back(
                localToGlobalIndex[localNeighbourIndex]);

        model.updateSDFGeometryNeighbours(globalIndex, neighboursTmp);
    }
    return {};
}
}
