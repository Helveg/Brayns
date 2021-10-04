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

#include "CircuitColorHandler.h"

#include <brayns/common/utils/stringUtils.h>
#include <brayns/engine/Material.h>

#include <plugin/api/ColorUtils.h>

#include <numeric>

namespace
{
inline std::vector<uint64_t> __parseIDRanges(const std::string& input)
{
    if(input.empty())
        throw std::invalid_argument("CircuitColorHandler: Received empty ID / ID range");

    std::vector<uint64_t> result;
    const auto dashPos = input.find("-");
    if(dashPos == std::string::npos)
    {
        try
        {
            result.push_back(std::stoull(input));
        }
        catch (...)
        {
            throw std::runtime_error("CircuitColorHandler: Could not parse ID '" + input + "'");
        }
    }
    else
    {
        const auto rangeBeginStr = input.substr(0, dashPos);
        const auto rangeEndStr = input.substr(dashPos + 1);
        try
        {
            const auto rangeStart = std::stoull(rangeBeginStr);
            const auto rangeEnd = std::stoull(rangeEndStr) + 1;

            if(rangeEnd <= rangeStart)
                throw std::invalid_argument("The range end must be greater than the range start");
            result.resize(rangeEnd - rangeStart);
            std::iota(result.begin(), result.end(), rangeStart);
        }
        catch (const std::invalid_argument& e)
        {
            throw std::runtime_error("CircuitColorHandler: Could not parse ID range '"+input+"': "
                                     + std::string(e.what()));
        }
    }

    return result;
}

inline void updateMaterialImpl(brayns::ModelDescriptor* model,
                               const size_t id,
                               const brayns::Vector4f& color)
{
    if(id == std::numeric_limits<size_t>::max())
        return;

    auto material = model->getModel().getMaterial(id);
    material->setDiffuseColor(brayns::Vector3d(color.r, color.g, color.b));
    material->setOpacity(static_cast<double>(color.a));
    material->markModified();
    material->commit();
}

} // namespace

void ElementMaterialMap::_updateMaterial(brayns::ModelDescriptor* model,
                                         const size_t id,
                                         const brayns::Vector4f& color) const
{
    ::updateMaterialImpl(model, id, color);
}

CircuitColorHandler::CircuitColorHandler(brayns::ModelDescriptor* model)
 : _model(model)
{
    if(!_model)
        throw std::invalid_argument("CircuitColorHandler: Null model passed");
}

void CircuitColorHandler::initialize()
{
    _methods = _getMethodsImpl();
    _methodVariables.resize(_methods.size());
}

void CircuitColorHandler::setElements(const std::vector<uint64_t>& ids,
                                      std::vector<ElementMaterialMap::Ptr>&& elements)
{
    _setElementsImpl(ids, std::move(elements));
}

const std::vector<std::string>& CircuitColorHandler::getMethods() const noexcept
{
    return _methods;
}

const std::vector<std::string>&
CircuitColorHandler::getMethodVariables(const std::string& method) const
{
    const auto lcm = brayns::string_utils::toLowercase(method);

    for(size_t i = 0; i < _methods.size(); ++i)
    {
        if(lcm == _methods[i])
        {
            auto& cache = _methodVariables[i];
            if(!cache.initialized)
            {
                cache.variables = _getMethodVariablesImpl(_methods[i]);
                cache.initialized = true;
            }
            return cache.variables;
        }
    }

    throw std::invalid_argument("CircuitColorHandler: Unknown method '" + method
                                + "' for model ID " + std::to_string(_model->getModelID()));
}

void CircuitColorHandler::updateColorById(const ColorVariables& variables)
{
    std::map<uint64_t, brayns::Vector4f> colorMap;
    if(!variables.empty())
    {
        for(const auto& entry : variables)
        {
            const auto& rawIds = entry.first;
            const auto& color = entry.second;

            const auto ids = __parseIDRanges(rawIds);
            for(const auto id : ids)
                colorMap[id] = color;
        }
    }

    _updateColorByIdImpl(colorMap);
    _model->markModified();
}

void CircuitColorHandler::updateColorById(const std::map<uint64_t, brayns::Vector4f>& colorMap)
{
    _updateColorByIdImpl(colorMap);
    _model->markModified();
}

void CircuitColorHandler::updateSingleColor(const brayns::Vector4f& color)
{
    _updateSingleColorImpl(color);
    _model->markModified();
}

void CircuitColorHandler::updateColor(const std::string& method, const ColorVariables& variables)
{
    const auto lcm = brayns::string_utils::toLowercase(method);

    auto it = std::find(_methods.begin(), _methods.end(), lcm);
    if(it == _methods.end())
        throw std::invalid_argument("CircuitColorHandler: Unknown method '" + method
                                    + "' for model ID " + std::to_string(_model->getModelID()));

    _updateColorImpl(lcm, variables);
    _model->markModified();
}

size_t CircuitColorHandler::getModelID() const noexcept
{
    return _model->getModelID();
}

void CircuitColorHandler::_updateMaterial(const size_t id, const brayns::Vector4f& color)
{
    ::updateMaterialImpl(_model, id, color);
}