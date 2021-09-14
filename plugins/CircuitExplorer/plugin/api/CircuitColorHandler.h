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

#include <map>
#include <unordered_map>
#include <vector>

using ColorVariables = std::unordered_map<std::string, brayns::Vector4f>;

/**
 * @brief The ElementMaterialMap class is the base class for the elemements that
 *        can be added to a circuit color handler. Must implement the logic to update
 *        all their relative materials to a given color.
 */
class ElementMaterialMap
{
public:
    using Ptr = std::unique_ptr<ElementMaterialMap>;

    virtual void setColor(brayns::ModelDescriptor* model,
                          const brayns::Vector4f& color) = 0;

protected:
    void _updateMaterial(brayns::ModelDescriptor* model,
                         const size_t id,
                         const brayns::Vector4f& color) const;
};

/**
 * @brief The MethodVariableCache struct holds the information of a specific
 *        method variables, with a parameter to allow lazy caching
 */
struct MethodVariableCache
{
    bool initialized {false};
    std::vector<std::string> variables;
};

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
    CircuitColorHandler(brayns::ModelDescriptor* model);

    /**
     * @brief initializeMethodInfo calls the subclasses methods to fill the cache of
     *        available color methods and method keys
     */
    void initialize();

    /**
     * @brief setElements allows to store the element material maps generated during
     *        the geoemtry add to model phase
     */
    void setElements(const std::vector<uint64_t>& ids,
                     std::vector<ElementMaterialMap::Ptr>&& elements);

    /**
     * @brief getAvailableMethods Return the available methods by which a circuit can be
     *        colored (For example: By ID, By layer, by population, ...)
     */
    const std::vector<std::string>& getMethods() const noexcept;

    /**
     * @brief getMethodVariables Return the possible variable specofications for a given
     *        method (For example, for layer it will return the list of loaded layers,
     *        for mtypes the list of loaded mtypes, ...)
     */
    const std::vector<std::string>& getMethodVariables(const std::string& method) const;

    /**
     * @brief updateColorById Updates color of the elements by the ID they are identified by.
     *        Specific ids and/or id ranges might be specified to isolate the update
     */
    void updateColorById(const ColorVariables& variables);

    /**
     * @brief updateColorById Updates color of the elements by the ID they are identified by.
     *        Specific ids might be specified to isolate the update
     */
    void updateColorById(const std::map<uint64_t, brayns::Vector4f>& colorMap);

    /**
     * @brief updateSingleColor Updates the color of all the elements to the given color
     */
    void updateSingleColor(const brayns::Vector4f& color);

    /**
     * @brief updateColor Updates the circuit color according to the given method. If one or
     *        more variables are specified, only these will be updated. Otherwise, updates
     *        the whole circuit.
     */
    void updateColor(const std::string& method, const ColorVariables& variables);

    size_t getModelID() const noexcept;

protected:
    void _updateMaterial(const size_t id, const brayns::Vector4f& color);

    brayns::ModelDescriptor* _model;
    std::vector<std::string> _methods;
    mutable std::vector<MethodVariableCache> _methodVariables;

protected:
    virtual void _setElementsImpl(const std::vector<uint64_t>& ids,
                                  std::vector<ElementMaterialMap::Ptr>&& elements) = 0;

    virtual std::vector<std::string> _getMethodsImpl() const = 0;

    virtual std::vector<std::string> _getMethodVariablesImpl(const std::string& method) const = 0;

    virtual void _updateColorByIdImpl(const std::map<uint64_t, brayns::Vector4f>& colorMap) = 0;

    virtual void _updateSingleColorImpl(const brayns::Vector4f& color) = 0;

    virtual void _updateColorImpl(const std::string& method, const ColorVariables& variables) = 0;
};
