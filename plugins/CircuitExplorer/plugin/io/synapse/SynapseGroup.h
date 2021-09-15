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

#include <plugin/api/CircuitColorHandler.h>
#include <plugin/io/morphology/MorphologyInstance.h>

#include <brayns/engine/Model.h>

#include <unordered_map>

class SynapseGroup
{
public:
    virtual ~SynapseGroup() = default;

    virtual void mapToCell(const MorphologyInstance&) = 0;
    virtual void mapSimulation(const std::unordered_map<uint64_t, uint64_t>&) = 0;
    virtual ElementMaterialMap::Ptr addToModel(brayns::Model& model) const = 0;
};
