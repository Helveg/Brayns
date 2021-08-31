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

#include <brayns/common/types.h>
#include <brayns/engine/Model.h>

#include <unordered_map>
#include <unordered_set>

using ColorVariables = std::unordered_map<std::string, brayns::Vector3f>;

/**
 * @brief The CircuitColorHandler class is in charge of providing an API to color neuronal
 *        circuits based on which parameters are available on the files from which they
 *        were loaded. Is a temporary solution that does the job without hardcoding
 *        it on the model/scene, until a comprehensive engine API for defining custom models
 *        is made available by the engine core
 */
class CircuitColorHandler
{
public:
    CircuitColorHandler(brayns::ModelDescriptor* model)
     : _model(model)
    {
    }

    /**
     * @brief getAvailableMethods Return the available methods by which a circuit can be
     *        colored (For example: By ID, By layer, by population, ...)
     */
    virtual std::unordered_set<std::string>
    getAvailableMethods() const noexcept = 0;

    /**
     * @brief getMethodVariables Return the possible variable specofications for a given
     *        method (For example, for layer it will return the list of loaded layers,
     *        for mtypes the list of loaded mtypes, ...)
     */
    virtual std::unordered_set<std::string>
    getMethodVariables(const std::string& method) const = 0;

    /**
     * @brief updateColor Updates the circuit color according to the given method. If one or
     *        more variables are specified, only these will be updated. Otherwise, updates
     *        the whole circuit.
     */
    virtual void
    updateColor(const std::string& method, const ColorVariables& variables) = 0;

protected:
    brayns::ModelDescriptor* _model;
};
