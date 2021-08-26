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

#include "PrimitiveNeuronBuilder.h"

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
} // namespace

void PrimitiveNeuronBuilder::build(const NeuronMorphology& m)
{
    _data = std::make_shared<PrimitiveSharedData>();

    // Add soma
    if(m.hasSoma())
    {
        const auto& soma = m.soma();
        addSphere(soma.center, soma.radius, NeuronSection::SOMA, -1);
        for(const auto child : soma.children)
        {
            if(!child->samples.empty())
                addCone(soma.center,
                        soma.radius,
                        brayns::Vector3f(*child->samples.begin()),
                        (*child->samples.begin()).w,
                        NeuronSection::SOMA,
                        -1);
        }
    }

    // Add dendrites and axon
    for(const auto& section : m.sections())
    {
        if(section.samples.size() > 1)
        {
            for (size_t i = 1; i < section.samples.size(); ++i)
            {
                const auto& s1 = section.samples[i - 1];
                const brayns::Vector3f p1(s1);
                const float r1 = s1.w;

                const auto& s2 = section.samples[i];
                const brayns::Vector3f p2(s2);
                const float r2 = s2.w;

                if (s1 != s2)
                {
                    if(almostEqual(r1, r2, 100000))
                        addCylinder(p1, p2, r1, section.type, section.id);
                    else
                        addCone(p1, r1, p2, r2, section.type, section.id);
                }
            }
        }
        else
        {
            const auto& s1 = section.samples.front();
            const brayns::Vector3f p1(s1);
            const float r1 = s1.w;
            addCylinder(p1, p1, r1, section.type, section.id);
        }
    }
}

std::unique_ptr<MorphologyInstance>
PrimitiveNeuronBuilder::instantiate(const brayns::Vector3f& t,
                                    const brayns::Quaternion& r) const
{
    auto sphereCopy = _spheres;
    for(auto& sphere : sphereCopy)
        sphere.center = t + r * sphere.center;

    auto coneCopy = _cones;
    for(auto& cone : coneCopy)
    {
        cone.center = t + r * cone.center;
        cone.up = t + r * cone.up;
    }

    auto cylinderCopy = _cylinders;
    for(auto& cylinder : cylinderCopy)
    {
        cylinder.center = t + r * cylinder.center;
        cylinder.up = t + r * cylinder.up;
    }

    return std::make_unique<PrimitiveNeuronInstance>(std::move(sphereCopy),
                                                     std::move(cylinderCopy),
                                                     std::move(coneCopy),
                                                     _data);
}

void PrimitiveNeuronBuilder::addSphere(const brayns::Vector3f& c,
                                       const float r,
                                       const NeuronSection section,
                                       const int32_t sectionId) noexcept
{
    const auto geomIdx = _data->geometries.size();
    const auto sphereIdx = _spheres.size();
    _spheres.push_back(brayns::Sphere(c, r));
    _data->geometries.push_back({PrimitiveType::SPHERE, sphereIdx});
    _data->sectionTypes.push_back(section);
    _data->sectionMap[sectionId].push_back(geomIdx);
}

void PrimitiveNeuronBuilder::addCylinder(const brayns::Vector3f& c,
                                           const brayns::Vector3f& u,
                                           const float r,
                                           const NeuronSection section,
                                           const int32_t sectionId) noexcept
{
    const auto geomIdx = _data->geometries.size();
    const auto cylinderIdx = _cylinders.size();
    _cylinders.push_back(brayns::Cylinder(c, u, r));
    _data->geometries.push_back({PrimitiveType::CYLINDER, cylinderIdx});
    _data->sectionTypes.push_back(section);
    _data->sectionMap[sectionId].push_back(geomIdx);
}

void PrimitiveNeuronBuilder::addCone(const brayns::Vector3f& c,
                                       const float r,
                                       const brayns::Vector3f& u,
                                       const float ru,
                                       const NeuronSection section,
                                       const int32_t sectionId) noexcept
{
    const auto geomIdx = _data->geometries.size();
    const auto coneIdx = _cones.size();
    _cones.push_back(brayns::Cone(c, u, r, ru));
    _data->geometries.push_back({PrimitiveType::CONE, coneIdx});
    _data->sectionTypes.push_back(section);
    _data->sectionMap[sectionId].push_back(geomIdx);
}
