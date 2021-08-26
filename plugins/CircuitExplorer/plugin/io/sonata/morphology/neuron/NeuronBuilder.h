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

#pragma once

#include <plugin/io/sonata/morphology/MorphologyInstance.h>
#include <plugin/io/sonata/morphology/neuron/NeuronMorphology.h>

#include <brayns/common/mathTypes.h>

/**
 * @brief The NeuronBuilder class is the base class to implement
 *        geometry builders that transform the Morphology object into a set
 *        of 3D shapes that are renderable. The result then is instantiated
 *        acording to each cell properties (position and rotation)
 */
class NeuronBuilder
{
public:
    virtual ~NeuronBuilder() = default;

    virtual void build(const NeuronMorphology&) = 0;

    virtual std::unique_ptr<MorphologyInstance> instantiate(const brayns::Vector3f&,
                                                            const brayns::Quaternion&) const = 0;
};
