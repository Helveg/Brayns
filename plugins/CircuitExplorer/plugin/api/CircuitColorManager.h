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

#include <vector>

#include <plugin/api/CircuitColorHandler.h>

/**
 * @brief The CircuitColorManager class is in charge of registering the color handlers of each
 *        loaded circuit and give access to them based on the model ID
 */
class CircuitColorManager
{
public:
    void registerHandler(std::unique_ptr<CircuitColorHandler>&& handler);
    void unregisterHandler(const size_t modelId);
    bool handlerExists(const size_t modelId) const noexcept;

    const std::vector<std::string>& getAvailableMethods(const uint64_t modelId) const;

    const std::vector<std::string>& getMethodVariables(const uint64_t modelId,
                                                       const std::string& method) const;

    void updateColorsById(const uint64_t modelId, const ColorVariables& variables);
    void updateColorsById(const uint64_t modelId,
                          const std::map<uint64_t, brayns::Vector4f>& colorMap);

    void updateSingleColor(const uint64_t modelId, const brayns::Vector4f& color);

    void updateColors(const uint64_t modelId, const std::string& method, const ColorVariables& vars);

private:
    std::vector<std::unique_ptr<CircuitColorHandler>> _handlers;
};
