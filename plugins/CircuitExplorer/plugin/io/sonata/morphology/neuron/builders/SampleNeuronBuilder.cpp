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

#include "SampleNeuronBuilder.h"

void SampleNeuronBuilder::build(const NeuronMorphology& m)
{
    _data = std::make_shared<SampleSharedData>();

    // Soma
    if(m.hasSoma())
    {
        const auto& soma = m.soma();
        _samples.push_back(brayns::Sphere(soma.center, soma.radius));
        _data->sectionTypes.push_back(NeuronSection::SOMA);
        _data->sectionMap[-1].push_back(0);
    }

    // Dendrites and axon
    for (const auto& section : m.sections())
    {
        for (const auto& sample : section.samples)
        {
            const auto idx = _samples.size();
            _samples.push_back(brayns::Sphere(brayns::Vector3f(sample), sample.w));
            _data->sectionTypes.push_back(section.type);
            _data->sectionMap[section.id].push_back(idx);
        }
    }
}

std::unique_ptr<MorphologyInstance>
SampleNeuronBuilder::instantiate(const brayns::Vector3f& t,
                                 const brayns::Quaternion& r) const
{
    auto transformed = _samples;
    for(auto& sphere : transformed)
        sphere.center = t + r * sphere.center;

    return std::make_unique<SampleNeuronInstance>(std::move(transformed), _data);
}
