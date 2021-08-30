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

#include <type_traits>
#include <unordered_map>
#include <unordered_set>

#include <plugin/api/CircuitColorHandler.h>

class CircuitColorManager
{
public:
    template<class Handler, typename ...Args>
    void registerHandler(brayns::ModelDescriptor* model, Args&&...args)
    {
        static_assert (std::is_base_of<CircuitColorHandler, Handler>::value,
                       "Attempted to register a non CircuitColorHandler class");

        _handlers[model->getModelID()] = std::make_unique<Handler>(std::forward<Args>(args)...);
    }

    std::unordered_set<std::string>
    getAvailableMethods(const uint64_t modelId);

    std::unordered_set<std::string>
    getMethodVariables(const uint64_t modelId, const std::string& method);

    void
    updateColors(const uint64_t modelId,
                 const std::string& method,
                 const ColorVariables& variables);

private:
    std::unordered_map<uint64_t, std::unique_ptr<CircuitColorHandler>> _handlers;
};
