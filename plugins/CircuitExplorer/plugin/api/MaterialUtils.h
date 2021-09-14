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

#include <cstdint>

#include <brayns/engine/Model.h>

#include <plugin/api/MaterialEnums.h>

/** Additional marterial attributes */
// TODO: Remove those once engines have been refactored
const std::string MATERIAL_PROPERTY_CAST_USER_DATA   = "cast_simulation_data";
const std::string MATERIAL_PROPERTY_SHADING_MODE     = "shading_mode";
const std::string MATERIAL_PROPERTY_CLIPPING_MODE    = "clipping_mode";
const std::string MATERIAL_PROPERTY_USER_PARAMETER   = "user_parameter";

class CircuitExplorerMaterial
{
public:
    static size_t create(brayns::Model& model,
                         const brayns::Vector3f& color = brayns::Vector3f(1.f, 1.f, 1.f));

    static void addExtraAttributes(brayns::Model& model);

    static void setSimulationColorEnabled(brayns::Model& model, const bool value);

private:
    static const brayns::PropertyMap& _getExtraAttributes() noexcept;
};
