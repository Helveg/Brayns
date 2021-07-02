/* Copyright (c) 2015-2021, EPFL/Blue Brain Project
 * All rights reserved. Do not distribute without permission.
 *
 * This file is part of the circuit explorer for Brayns
 * <https://github.com/BlueBrain/Brayns>
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

#include <brayns/common/types.h>

struct CellGeometryMap
{
    size_t somaMaterial {brayns::NO_MATERIAL};
    size_t axonMaterial {brayns::NO_MATERIAL};
    size_t dendriteMatrial {brayns::NO_MATERIAL};
    size_t apicalDendriteMaterial {brayns::NO_MATERIAL};
    std::vector<size_t> afferentSynMaterials;
    std::vector<size_t> efferentSynMaterials;
};
