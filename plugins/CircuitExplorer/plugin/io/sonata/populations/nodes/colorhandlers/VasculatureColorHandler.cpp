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

#include "VasculatureColorHandler.h"

#include <brayns/engine/Material.h>

#include <plugin/api/ColorUtils.h>

namespace
{
constexpr char methodBySection[]    = "vasculature_section";

constexpr char sectionVein[]        = "vein";
constexpr char sectionArtery[]      = "artery";
constexpr char sectionVenule[]      = "venule";
constexpr char sectionArteriole[]   = "arteriole";
constexpr char sectionVenousCap[]   = "venous_capillary";
constexpr char sectionArterialCap[] = "arterial_capillary";
constexpr char sectionTransitional[]= "transitional";

inline VasculatureSection strToVasculatureSection(const std::string& sectionStr)
{
    if(sectionStr.empty())
        throw std::invalid_argument("VasculatureColorHandler: Received empty section name");

    if(sectionStr == sectionVein)
        return VasculatureSection::VEIN;
    if(sectionStr == sectionArtery)
        return VasculatureSection::ARTERY;
    if(sectionStr == sectionVenule)
        return VasculatureSection::VENULE;
    if(sectionStr == sectionArteriole)
        return VasculatureSection::ARTERIOLE;
    if(sectionStr == sectionVenousCap)
        return VasculatureSection::VENOUS_CAPILLARY;
    if(sectionStr == sectionArterialCap)
        return VasculatureSection::ARTERIAL_CAPILLARY;
    if(sectionStr == sectionTransitional)
        return VasculatureSection::TRANSITIONAL;

    throw std::invalid_argument("VasculatureColorHandler: Unknown section type '"+sectionStr+"'");
}
}

void VasculatureMaterialMap::setColor(brayns::ModelDescriptor* model,
                                      const brayns::Vector3f& color)
{
    _updateMaterial(model, materialId, color);
}

VasculatureColorHandler::VasculatureColorHandler(brayns::ModelDescriptor* model,
                                                 const std::string& configPath,
                                                 const std::string& population)
 : PopulationColorHandler(model, configPath, population)
{
}

void VasculatureColorHandler::_setElementsImpl(const std::vector<uint64_t>& ids,
                                               std::vector<ElementMaterialMap::Ptr>&& elements)
{
    _ids = ids;
    _elements = std::move(elements);
    for(auto& element : _elements)
    {
        auto& vmm = static_cast<VasculatureMaterialMap&>(*element.get());
        _sectionMaterials[vmm.sectionType].push_back(vmm.materialId);
    }
}

std::vector<std::string> VasculatureColorHandler::_getMethodsImpl()
{
    return {methodBySection};
}

std::vector<std::string>
VasculatureColorHandler::_getMethodVariablesImpl(const std::string& method)
{
    return {sectionVein, sectionArtery, sectionVenule, sectionArteriole,
            sectionVenousCap, sectionArterialCap, sectionTransitional};
}

void
VasculatureColorHandler::_updateColorByIdImpl(const std::map<uint64_t, brayns::Vector3f>& colorMap)
{
    if(!colorMap.empty())
    {
        auto it = colorMap.begin();
        size_t i = 0;
        while(it != colorMap.end() && i < _ids.size())
        {
            const auto id = it->first;
            while(_ids[i] < id && i < _ids.size())
                ++i;

            if(_ids[i] == id)
                _elements[i]->setColor(_model, it->second);
            else
                throw std::invalid_argument("NeuronColorHandler: Could not set color by ID: ID '"
                                            + std::to_string(id) + "' not found in circuit");

            ++i;
            ++it;
        }
    }
    else
    {
        ColorRoulette r;
        for(auto& element : _elements)
            element->setColor(_model, r.getNextColor());
    }
}

void VasculatureColorHandler::_updateSingleColorImpl(const brayns::Vector3f& color)
{
    for(auto& element : _elements)
        element->setColor(_model, color);
}

void VasculatureColorHandler::_updateColorImpl(const std::string& method,
                                               const ColorVariables& variables)
{
    if(!variables.empty())
    {
        for(const auto& entry : variables)
        {
            const auto sectionType = strToVasculatureSection(entry.first);
            auto sectionMaterials = _sectionMaterials.find(sectionType);
            if(sectionMaterials != _sectionMaterials.end())
            {
                for(const auto materialId : sectionMaterials->second)
                    _updateMaterial(materialId, entry.second);
            }
        }
    }
    else
    {
        const std::vector<VasculatureSection> allSections =
        {
            VasculatureSection::ARTERIAL_CAPILLARY,
            VasculatureSection::ARTERIOLE,
            VasculatureSection::ARTERY,
            VasculatureSection::TRANSITIONAL,
            VasculatureSection::VEIN,
            VasculatureSection::VENOUS_CAPILLARY,
            VasculatureSection::VENULE
        };

        ColorRoulette roulette;
        for(const auto& section : allSections)
        {
            auto sectionMaterials = _sectionMaterials.find(section);
            if(sectionMaterials != _sectionMaterials.end())
            {
                const auto& color = roulette.getNextColor();
                for(const auto materialId : sectionMaterials->second)
                    _updateMaterial(materialId, color);
            }
        }
    }
}
