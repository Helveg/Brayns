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
                                const std::vector<std::unique_ptr<CircuitColorHandler>>& handlers)
{
    for(auto& handler : handlers)
    {
        if(handler->getModelID() == modelId)
            return handler.get();
    }

    throw std::invalid_argument("CircuitColorManager: Model ID '"
                                + std::to_string(modelId) + " not registered");
}
}

void CircuitColorManager::registerHandler(std::unique_ptr<CircuitColorHandler>&& handler)
{
    handler->initialize();
    _handlers.push_back(std::move(handler));
}

void CircuitColorManager::unregisterHandler(const size_t modelId)
{
    auto it = _handlers.begin();
    while(it != _handlers.end())
    {
        if((*it)->getModelID() == modelId)
        {
            _handlers.erase(it);
            break;
        }
        else
            ++it;
    }
}

bool CircuitColorManager::handlerExists(const size_t modelId) const noexcept
{
    for(const auto& handler : _handlers)
    {
        if(handler->getModelID() == modelId)
            return true;
    }

    return false;
}

const std::vector<std::string>&
CircuitColorManager::getAvailableMethods(const uint64_t modelId) const
{
    return getHandler(modelId, _handlers)->getMethods();
}

const std::vector<std::string>&
CircuitColorManager::getMethodVariables(const uint64_t modelId, const std::string& method) const
{
    return getHandler(modelId, _handlers)->getMethodVariables(method);
}

void
CircuitColorManager::updateColorsById(const uint64_t modelId, const ColorVariables& variables)
{
    getHandler(modelId, _handlers)->updateColorById(variables);
}

void
CircuitColorManager::updateColorsById(const uint64_t modelId,
                                      const std::map<uint64_t, brayns::Vector4f>& colorMap)
{
    getHandler(modelId, _handlers)->updateColorById(colorMap);
}

void
CircuitColorManager::updateSingleColor(const uint64_t modelId, const brayns::Vector4f& color)
{
    getHandler(modelId, _handlers)->updateSingleColor(color);
}

void
CircuitColorManager::updateColors(const uint64_t modelId,
                                  const std::string& method,
                                  const ColorVariables& vars)
{
    getHandler(modelId, _handlers)->updateColor(method, vars);
}
