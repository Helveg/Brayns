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

#include "VasculatureSDFBuilder.h"

namespace
{
// From http://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
template <class T>
typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
almostEqual(T x, T y, int ulp)
{
    // the machine epsilon has to be scaled to the magnitude of the values used
    // and multiplied by the desired precision in ULPs (units in the last place)
    return std::abs(x - y) <=
               std::numeric_limits<T>::epsilon() * std::abs(x + y) * ulp
           // unless the result is subnormal
           || std::abs(x - y) < std::numeric_limits<T>::min();
}

class Builder
{
public:
    struct Geometry
    {
        std::vector<brayns::SDFGeometry> objects;
        std::vector<std::unordered_set<size_t>> neighbours;
        std::vector<VasculatureSection> sectionTypes;
        std::unordered_map<int32_t, std::vector<size_t>> sections;
    };

    Builder(const VasculatureMorphology& m)
     : _morph(m)
    {
        build();
    }

    Geometry& getGeometry() noexcept
    {
        return _geometry;
    }

private:
    void build()
    {
        // Dendrites and axon
        for (const auto& section : _morph.sections())
        {
            std::vector<size_t> sectionGeoms;
            sectionGeoms.reserve(section.segments.size());
            for (const auto& segment : section.segments)
            {
                if (segment.start != segment.end)
                    sectionGeoms.push_back(_addSegment(segment.end, segment.endRadius,
                                                       segment.start, segment.startRadius,
                                                       section.id, section.type));
            }


            // Update geometry hierarchy use to merge bifurcations at the end
            const auto parentId = section.parentId == -1? section.id : section.parentId;
            _sectionHierarchy[parentId].push_back(sectionGeoms.front());
            _sectionHierarchy[section.id].push_back(sectionGeoms.back());
        }

        connectSDFBifurcations();
    }

    size_t _addSDFGeometry(brayns::SDFGeometry&& geometry,
                           const int32_t section,
                           const VasculatureSection type)
    {
        const size_t idx = _geometry.objects.size();
        _geometry.objects.push_back(geometry);
        _geometry.neighbours.emplace_back();
        _geometry.sectionTypes.push_back(type);
        _geometry.sections[section].push_back(idx);
        return idx;
    }

    size_t _addSegment(const brayns::Vector3f& pos,
                       const float radius,
                       const brayns::Vector3f& target,
                       const float targetRadius,
                       const int32_t section,
                       const VasculatureSection type)
    {
        brayns::SDFGeometry geom = (almostEqual(radius, targetRadius, 100000))
                ? brayns::createSDFPill(pos, target, radius)
                : brayns::createSDFConePill(pos, target, radius, targetRadius);

        return _addSDFGeometry(std::move(geom), section, type);
    }

    void connectSDFBifurcations()
    {
        for(const auto& entry : _sectionHierarchy)
        {
            for(const auto geomIdx : entry.second)
            {
                auto& geomNeighbours = _geometry.neighbours[geomIdx];
                for(const auto neighIdx : entry.second)
                    geomNeighbours.insert(neighIdx);
                geomNeighbours.erase(geomIdx);
            }
        }
    }


private:
    const VasculatureMorphology& _morph;
    Geometry _geometry;
    // Maps bifurcation section parent to the geometry indices taking
    // part in surch bifurcation, so they can be linked together
    std::unordered_map<int32_t, std::vector<size_t>> _sectionHierarchy;
};
}

std::unique_ptr<VasculatureSDFInstance>
VasculatureSDFBuilder::build(const VasculatureMorphology& m) const
{
    Builder b(m);
    auto& geometry = b.getGeometry();

    auto instanceGeometry = std::make_shared<VasculatureSDFInstance::Geometry>();
    instanceGeometry->objects = std::move(geometry.objects);

    instanceGeometry->neighbours.resize(geometry.neighbours.size());
    for(size_t i = 0; i < geometry.neighbours.size(); ++i)
    {
        const auto& ns = geometry.neighbours[i];
        instanceGeometry->neighbours[i] = std::vector<size_t>(ns.begin(), ns.end());
    }
    instanceGeometry->sectionTypes = std::move(geometry.sectionTypes);
    instanceGeometry->sections = std::move(geometry.sections);

    return std::make_unique<VasculatureSDFInstance>(instanceGeometry);
}
