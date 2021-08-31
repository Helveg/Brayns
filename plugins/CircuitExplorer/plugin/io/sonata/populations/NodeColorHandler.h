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

#include <unordered_map>

struct ElementMaterialMap
{
    using Ptr = std::unique_ptr<ElementMaterialMap>;
};

class NodeColorHandler : public CircuitColorHandler
{
public:
    NodeColorHandler(brayns::ModelDescriptor* model,
                     const std::string& configPath,
                     const std::string& population)
     : CircuitColorHandler(model)
     , _configPath(configPath)
     , _population(population)
    {
    }

    void addElement(const uint64_t id, ElementMaterialMap::Ptr&& map) noexcept
    {
        _maps.push_back(std::move(map));
        _materialMap[id] = _maps.back().get();
    }

protected:
    const std::string _configPath;
    const std::string _population;

    std::unordered_map<uint64_t, ElementMaterialMap*> _materialMap;
    std::vector<ElementMaterialMap::Ptr> _maps;
};
