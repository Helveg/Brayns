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

#include "CircuitColorManager.h"

namespace
{
CircuitColorHandler* getHandler(const uint64_t modelId,
                                std::unordered_map<uint64_t,
                                std::unique_ptr<CircuitColorHandler>>& handlers)
{
    auto it = handlers.find(modelId);
    if(it == handlers.end())
        throw std::runtime_error("CircuitColorManager: Model ID " + std::to_string(modelId)
                                 + " not registered");

    return it->second.get();
}
}

std::unordered_set<std::string>
CircuitColorManager::getAvailableMethods(const uint64_t modelId)
{
    return getHandler(modelId, _handlers)->getAvailableMethods();
}

std::unordered_set<std::string>
CircuitColorManager::getMethodVariables(const uint64_t modelId, const std::string& method)
{
    return getHandler(modelId, _handlers)->getMethodVariables(method);
}

void
CircuitColorManager::updateColors(const uint64_t modelId,
                                  const std::string& method,
                                  const ColorVariables& variables)
{
    getHandler(modelId, _handlers)->updateColor(method, variables);
}
