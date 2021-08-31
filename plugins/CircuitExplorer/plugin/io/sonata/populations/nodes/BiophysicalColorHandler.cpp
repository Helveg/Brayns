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

#include <numeric>

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

std::vector<uint64_t> parseNodeRanges(const std::string& input)
{
    if(input.empty())
        throw std::invalid_argument("ColorHandler: Received empty node ID / node range ID");

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
            throw std::runtime_error("ColorHandler: Could not parse node ID '" + input + "'");
        }
    }
    else
    {
        const auto rangeBeginStr = input.substr(0, dashPos);
        const auto rangeEndStr = input.substr(dashPos + 1);
        try
        {
            const auto rangeStart = std::stoull(rangeBeginStr);
            const auto rangeEnd = std::stoull(rangeEndStr);
            result.resize(rangeEnd - rangeStart);
            std::iota(result.begin(), result.end(), rangeStart);
        }
        catch (...)
        {
            throw std::runtime_error("ColorHandler: Could not parse node range ID '"
                                     + input + "'");
        }
    }
}

void colorWithInput(brayns::ModelDescriptor* model,
                    std::unordered_map<uint64_t, ElementMaterialMap*>& mapping,
                    const std::string& method,
                    const ColorVariables& input,
                    const std::string& configPath,
                    const std::string& population)
{
    if(method == "node_id")
    {
        for(const auto& entry : input)
        {
            std::vector<uint64_t> nodeIds = parseNodeRanges(entry.first);
            for(const auto nodeId : nodeIds)
            {
                auto& element = static_cast<BiophysicalMaterialMap&>(*mapping[nodeId]);
                updateMaterial(model, element.soma, entry.second);
                updateMaterial(model, element.axon, entry.second);
                updateMaterial(model, element.dendrite, entry.second);
                updateMaterial(model, element.apicalDendrite, entry.second);
            }
        }
    }
    else if(method == "morphology_part")
    {
        const auto somaColorIt = input.find("soma");
        if(somaColorIt != input.end())
        {
            for(auto& elementMap : mapping)
            {
                auto& element = static_cast<BiophysicalMaterialMap&>(*elementMap.second);
                updateMaterial(model, element.soma, somaColorIt->second);
            }
        }
        const auto axonColorIt = input.find("axon");
        if(axonColorIt != input.end())
        {
            for(auto& elementMap : mapping)
            {
                auto& element = static_cast<BiophysicalMaterialMap&>(*elementMap.second);
                updateMaterial(model, element.axon, axonColorIt->second);
            }
        }
        const auto dendColorIt = input.find("dendrite");
        if(dendColorIt != input.end())
        {
            for(auto& elementMap : mapping)
            {
                auto& element = static_cast<BiophysicalMaterialMap&>(*elementMap.second);
                updateMaterial(model, element.dendrite, dendColorIt->second);
            }
        }
        const auto aDendColorIt = input.find("apical_dendrite");
        if(aDendColorIt != input.end())
        {
            for(auto& elementMap : mapping)
            {
                auto& element = static_cast<BiophysicalMaterialMap&>(*elementMap.second);
                updateMaterial(model, element.apicalDendrite, aDendColorIt->second);
            }
        }
    }
    else
    {
        const auto nodeIds = getNodeIds(mapping);
        const auto selection = bbp::sonata::Selection::fromValues(nodeIds);
        const auto values = getVariables(configPath, population, method, selection);

        std::unordered_map<std::string, std::vector<uint64_t>> map;
        for(size_t i = 0; i < values.size(); ++i)
        {
            map[values[i]].push_back(nodeIds[i]);
        }
        for(const auto& entry : map)
        {
            const auto& color = input.at(entry.first);
            for(const auto nodeId : entry.second)
            {
                auto& element = static_cast<BiophysicalMaterialMap&>(*mapping[nodeId]);
                updateMaterial(model, element.soma, color);
                updateMaterial(model, element.axon, color);
                updateMaterial(model, element.dendrite, color);
                updateMaterial(model, element.apicalDendrite, color);
            }
        }
    }
}

void colorRandomly(brayns::ModelDescriptor* model,
                   std::vector<ElementMaterialMap::Ptr>& maps,
                   const std::unordered_map<uint64_t, ElementMaterialMap*>& materialMap,
                   const std::string& method,
                   const std::string& configPath,
                   const std::string& population)
{
    if(method == "node_id")
    {
        ColorRoulette colors;
        for(auto& elementPtr : maps)
        {
            const auto& nextColor = colors.getNextColor();
            auto& element = static_cast<BiophysicalMaterialMap&>(*elementPtr.get());
            updateMaterial(model, element.soma, nextColor);
            updateMaterial(model, element.axon, nextColor);
            updateMaterial(model, element.dendrite, nextColor);
            updateMaterial(model, element.apicalDendrite, nextColor);
        }
    }
    else if(method == "morphology_part")
    {
        ColorRoulette colors;
        const auto& somaColor = colors.getNextColor();
        const auto& axonColor = colors.getNextColor();
        const auto& dendColor = colors.getNextColor();
        const auto& apicalDendColor = colors.getNextColor();
        for(auto& elementPtr : maps)
        {
            auto& element = static_cast<BiophysicalMaterialMap&>(*elementPtr.get());
            updateMaterial(model, element.soma, somaColor);
            updateMaterial(model, element.axon, axonColor);
            updateMaterial(model, element.dendrite, dendColor);
            updateMaterial(model, element.apicalDendrite, apicalDendColor);
        }
    }
    else
    {
        const auto nodeIds = getNodeIds(materialMap);
        const auto selection = bbp::sonata::Selection::fromValues(nodeIds);
        const auto values = getVariables(configPath, population, method, selection);

        ColorDeck deck;

        for(size_t i = 0; i < maps.size(); ++i)
        {
            auto& element = static_cast<BiophysicalMaterialMap&>(*maps[i].get());
            const auto& key = values[i];
            const auto& color = deck.getColorForKey(key);
            updateMaterial(model, element.soma, color);
            updateMaterial(model, element.axon, color);
            updateMaterial(model, element.dendrite, color);
            updateMaterial(model, element.apicalDendrite, color);
        }
    }
}

} // namespace

BiophysicalColorHandler::BiophysicalColorHandler(brayns::ModelDescriptor* model,
                                                 const std::string& configPath,
                                                 const std::string& population)
 : NodeColorHandler(model, configPath, population)
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
        colorWithInput(_model, _materialMap, method, variables, _configPath, _population);
    else
        colorRandomly(_model, _maps, _materialMap, method, _configPath, _population);
}
