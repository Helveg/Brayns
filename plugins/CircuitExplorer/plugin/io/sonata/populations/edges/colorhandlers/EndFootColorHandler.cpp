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

#include "EndFootColorHandler.h"

#include <plugin/api/ColorUtils.h>

void EndFootColorHandler::_setElementsImpl(const std::vector<uint64_t>& ids,
                                           std::vector<ElementMaterialMap::Ptr>&& elements)
{
    _materials.reserve(elements.size());
    for(const auto& element : elements)
    {
        const auto& emm = static_cast<const EndFootMaterialMap&>(*element.get());
        _materials.insert(_materials.end(), emm.materials.begin(), emm.materials.end());
    }
}

void
EndFootColorHandler::_updateColorByIdImpl(const std::map<uint64_t, brayns::Vector3f>& colorMap)
{
    ColorRoulette r;
    for(const auto matId : _materials)
        _updateMaterial(matId, r.getNextColor());
}

void EndFootColorHandler::_updateSingleColorImpl(const brayns::Vector3f& color)
{
    for(const auto matId : _materials)
        _updateMaterial(matId, color);
}
