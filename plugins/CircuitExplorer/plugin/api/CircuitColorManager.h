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
    /**
     * @brief registerHandler registers a CircuitColorHandler to make it available to
     *        access
     */
    void registerHandler(std::unique_ptr<CircuitColorHandler>&& handler);

    /**
     * @brief unregisterHandler destroys the handler associated with the given model ID
     *        (if any), effectively releasing all the resources associated with it
     */
    void unregisterHandler(const size_t modelId);

    /**
     * @brief handlerExists checks wether the given model ID has a CircuitColorHandler
     *        associated with it
     */
    bool handlerExists(const size_t modelId) const noexcept;

    /**
     * @brief getAvailableMethods return the available extra coloring methods from
     *        the CircuitColorHandler associated with the given model ID.
     * @throws std::runtime_error if there is no handler associated with the given ID
     */
    const std::vector<std::string>& getAvailableMethods(const uint64_t modelId) const;

    /**
     * @brief getMethodVariables return the available variables for the given method
     *        from the handler associated with the given ID
     * @throws std::runtime_error if the model ID does not have a handler associated,
     *         or if the associated handler does not have the given method
     */
    const std::vector<std::string>& getMethodVariables(const uint64_t modelId,
                                                       const std::string& method) const;

    /**
     * @brief updateColorsById calls the handler associated with the modelId to color
     *        its elements by id, with the optional id/id ranges specified via variables
     * @throws std::runtime_error if the given model ID does not have a handler associated,
     *         or if the id/id ranges specified (if any) cannot be parsed
     */
    void updateColorsById(const uint64_t modelId, const ColorVariables& variables);

    /**
     * @brief updateColorsById calls the handler associated with the modelId to color
     *        its elements by id, with the optional id specified via colorMap
     * @throws std::runtime_error if the given model ID does not have a handler associated
     */
    void updateColorsById(const uint64_t modelId,
                          const std::map<uint64_t, brayns::Vector4f>& colorMap);

    /**
     * @brief updateSingleColor calls the handler associated with the modelId to color
     *        all its elements with the same specified color.
     * @throws std::runtime_error if the given model ID does not have a handler associated
     */
    void updateSingleColor(const uint64_t modelId, const brayns::Vector4f& color);

    /**
     * @brief updateColors calls the handler associated with the modelId to color all
     *        its elements using the specified method, optionally targeting specific ones
     *        via the variables.
     * @throws std::runtime_error if the given model ID does not have a handler associated,
     *         or if the associated handler does not have the given method
     */
    void updateColors(const uint64_t modelId, const std::string& method, const ColorVariables& vars);

private:
    std::vector<std::unique_ptr<CircuitColorHandler>> _handlers;
};
