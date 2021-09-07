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

#include <plugin/io/sonata/SonataLoaderEnums.h>
#include <plugin/io/sonata/populations/PopulationColorHandler.h>

#include <unordered_map>

class VasculatureMaterialMap : public ElementMaterialMap
{
public:
    size_t materialId;
    VasculatureSection sectionType;

    void setColor(brayns::ModelDescriptor* model,
                  const brayns::Vector3f& color) final;
};

class VasculatureColorHandler : public PopulationColorHandler
{
public:
    VasculatureColorHandler(brayns::ModelDescriptor* model,
                            const std::string& configPath,
                            const std::string& population);

    void _setElementsImpl(const std::vector<uint64_t>& ids,
                          std::vector<ElementMaterialMap::Ptr>&& elements) final;

    std::vector<std::string> _getMethodsImpl() final;

    std::vector<std::string> _getMethodVariablesImpl(const std::string& method) final;

    void _updateColorByIdImpl(const std::map<uint64_t, brayns::Vector3f>& colorMap) final;

    void _updateSingleColorImpl(const brayns::Vector3f& color) final;

    void _updateColorImpl(const std::string& method, const ColorVariables& variables) final;

private:
    std::vector<uint64_t> _ids;
    std::vector<ElementMaterialMap::Ptr> _elements;
    std::unordered_map<VasculatureSection, std::vector<size_t>> _sectionMaterials;
};
