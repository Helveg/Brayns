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

#include <plugin/io/sonata/populations/PopulationColorHandler.h>

class EndFootMaterialMap : public ElementMaterialMap
{
public:
    std::vector<size_t> materials;

    void setColor(brayns::ModelDescriptor* model, const brayns::Vector3f& color)
    {
    }
};

class EndFootColorHandler : public EdgePopulationColorHandler
{
public:
    EndFootColorHandler(brayns::ModelDescriptor* model,
                        const std::string& configPath,
                        const std::string& population,
                        const bool afferent)
     : EdgePopulationColorHandler(model, configPath, population, afferent)
    {
    }

    void _setElementsImpl(const std::vector<uint64_t>& ids,
                          std::vector<ElementMaterialMap::Ptr>&& elements) final;

    std::vector<std::string> _getMethodsImpl() final
    {
        return {};
    }

    std::vector<std::string> _getMethodVariablesImpl(const std::string& method)
    {
        return {};
    }

    void _updateColorByIdImpl(const std::map<uint64_t, brayns::Vector3f>& colorMap) final;

    void _updateSingleColorImpl(const brayns::Vector3f& color) final;

    void _updateColorImpl(const std::string& method, const ColorVariables& variables) final
    {
    }

private:
    std::vector<size_t> _materials;
};
