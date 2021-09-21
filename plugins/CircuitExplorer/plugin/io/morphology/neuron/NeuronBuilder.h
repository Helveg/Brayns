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

#include <plugin/io/morphology/MorphologyInstance.h>
#include <plugin/io/morphology/neuron/NeuronMorphology.h>

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

    /**
     * @brief builds the geometry from the given morphology representation. If the builders
     *        has already been used to build a geometry, this function has no effect
     */
    void build(const NeuronMorphology& nm)
    {
        if(!_initialized)
        {
            _buildImpl(nm);
            _initialized = true;
        }
    }

    /**
     * @brief creates a morphology instance by transforming the geometry built in the build()
     *        method with the given translation and rotation
     */
    MorphologyInstancePtr instantiate(const brayns::Vector3f& t,
                                      const brayns::Quaternion& r) const
    {
        return _instantiateImpl(t, r);
    }

protected:
    virtual void _buildImpl(const NeuronMorphology&) = 0;

    virtual MorphologyInstancePtr _instantiateImpl(const brayns::Vector3f&,
                                                   const brayns::Quaternion&) const = 0;

private:
    bool _initialized {false};
};
