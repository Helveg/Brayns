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

#include "BiophysicalColorHandler.h"

#include <bbp/sonata/config.h>

#include <brayns/engine/Material.h>

#include <plugin/api/ColorUtils.h>

namespace
{
std::unordered_set<std::string> fillMethods(const std::string& configP, const std::string& pop)
{
    const bbp::sonata::CircuitConfig config = bbp::sonata::CircuitConfig::fromFile(configP);
    const auto population = config.getNodePopulation(pop);
    const auto& attributes = population.attributeNames();

    const std::vector<std::string> possibleMethods = {"morphology", "layer", "morph_class",
                                                      "etype", "mtype", "synapse_class",
                                                      "region", "hemisphere"};

    std::vector<std::string> result;
    result.reserve(possibleMethods.size() + 1);
    result.push_back("node_id");
    result.push_back("morphology_section");

    for(const auto& possible : possibleMethods)
    {
        if(attributes.find(possible) != attributes.end())
            result.push_back(possible);
    }

    return std::unordered_set<std::string>(result.begin(), result.end());
}

inline std::vector<uint64_t>
getNodeIds(const std::unordered_map<uint64_t, ElementMaterialMap*>& map)
{
    std::vector<uint64_t> nodes;
    nodes.reserve(map.size());
    for(const auto& entry : map)
        nodes.push_back(entry.first);
    return nodes;
}

std::vector<std::string> getVariables(const std::string& configP,
                                      const std::string& pop,
                                      const std::string& method,
                                      const bbp::sonata::Selection& selection)
{
    const bbp::sonata::CircuitConfig config = bbp::sonata::CircuitConfig::fromFile(configP);
    const auto population = config.getNodePopulation(pop);
    const auto& attributes = population.attributeNames();

    if(attributes.find(method) == attributes.end())
        throw std::invalid_argument("Population " + pop + " has no attribute " + method);

    const auto loadedVariables = population.getAttribute<std::string>(method, selection);

    return loadedVariables;
}

inline void updateMaterial(brayns::ModelDescriptor* model,
                           const size_t materialId,
                           const brayns::Vector3f& newColor)
{
    if(materialId == std::numeric_limits<size_t>::max())
        return;

    model->getModel().getMaterial(materialId)->setDiffuseColor(newColor);
}

}

BiophysicalColorHandler::BiophysicalColorHandler(brayns::ModelDescriptor* model,
                                                 const std::string& configPath,
                                                 const std::string& population)
 : SonataCircuitColorHandler(model, configPath, population)
 , _methodCache(fillMethods(_configPath, _population))
{
}


std::unordered_set<std::string> BiophysicalColorHandler::getAvailableMethods() const noexcept
{
    return _methodCache;
}

std::unordered_set<std::string>
BiophysicalColorHandler::getMethodVariables(const std::string& method) const
{
    if(method == "node_id")
        return {};

    if(method == "morphology_section")
        return {"soma", "axon", "dendrite", "apical_dendrite"};

    const auto selection = bbp::sonata::Selection::fromValues(getNodeIds(_materialMap));
    const auto values = getVariables(_configPath,
                                     _population,
                                     method,
                                     selection);
    return std::unordered_set<std::string>(values.begin(), values.end());
}

void
BiophysicalColorHandler::updateColor(const std::string& method, const ColorVariables& variables)
{
    if(!variables.empty())
    {
        if(method == "node_id")
        {
            for(const auto& entry : variables)
            {
                const auto nodeId = std::stoull(entry.first);
                auto& element = static_cast<BiophysicalMaterialMap&>(*_materialMap[nodeId]);
                updateMaterial(_model, element.soma, entry.second);
                updateMaterial(_model, element.axon, entry.second);
                updateMaterial(_model, element.dendrite, entry.second);
                updateMaterial(_model, element.apicalDendrite, entry.second);
            }
        }
        else if(method == "morphology_part")
        {
            const auto somaColorIt = variables.find("soma");
            if(somaColorIt != variables.end())
            {
                for(auto& elementMap : _materialMap)
                {
                    auto& element = static_cast<BiophysicalMaterialMap&>(*elementMap.second);
                    updateMaterial(_model, element.soma, somaColorIt->second);
                }
            }
            const auto axonColorIt = variables.find("axon");
            if(axonColorIt != variables.end())
            {
                for(auto& elementMap : _materialMap)
                {
                    auto& element = static_cast<BiophysicalMaterialMap&>(*elementMap.second);
                    updateMaterial(_model, element.axon, axonColorIt->second);
                }
            }
            const auto dendColorIt = variables.find("dendrite");
            if(dendColorIt != variables.end())
            {
                for(auto& elementMap : _materialMap)
                {
                    auto& element = static_cast<BiophysicalMaterialMap&>(*elementMap.second);
                    updateMaterial(_model, element.dendrite, dendColorIt->second);
                }
            }
            const auto aDendColorIt = variables.find("apical_dendrite");
            if(aDendColorIt != variables.end())
            {
                for(auto& elementMap : _materialMap)
                {
                    auto& element = static_cast<BiophysicalMaterialMap&>(*elementMap.second);
                    updateMaterial(_model, element.apicalDendrite, aDendColorIt->second);
                }
            }
        }
        else
        {
            const auto nodeIds = getNodeIds(_materialMap);
            const auto selection = bbp::sonata::Selection::fromValues(nodeIds);
            const auto values = getVariables(_configPath, _population, method, selection);

            std::unordered_map<std::string, std::vector<uint64_t>> map;
            for(size_t i = 0; i < values.size(); ++i)
            {
                map[values[i]].push_back(nodeIds[i]);
            }
            for(const auto& entry : map)
            {
                const auto& color = variables.at(entry.first);
                for(const auto nodeId : entry.second)
                {
                    auto& element = static_cast<BiophysicalMaterialMap&>(*_materialMap[nodeId]);
                    updateMaterial(_model, element.soma, color);
                    updateMaterial(_model, element.axon, color);
                    updateMaterial(_model, element.dendrite, color);
                    updateMaterial(_model, element.apicalDendrite, color);
                }
            }
        }
    }
    else
    {

    }
}
