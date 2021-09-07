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

#include "NeuronColorHandler.h"

#include <brayns/engine/Material.h>

#include <plugin/api/ColorUtils.h>

#include <unordered_set>

namespace
{
constexpr char methodBySection[]        = "morphology_section";
constexpr char methodByMorphology[]     = "morphology";
constexpr char methodByLayer[]          = "layer";
constexpr char methodByMorphClass[]     = "morph_class";
constexpr char methodByEType[]          = "etype";
constexpr char methodByMType[]          = "mtype";
constexpr char methodBySynapseClass[]   = "synapse_class";
constexpr char methodByRegion[]         = "region";
constexpr char methodByHemisphere[]     = "hemisphere";

constexpr char sectionSoma[]            = "soma";
constexpr char sectionAxon[]            = "axon";
constexpr char sectionDendrite[]        = "dendrite";
constexpr char sectionApicalDendrite[]  = "apical_dendrite";

std::vector<std::string> getVariables(const bbp::sonata::CircuitConfig& config,
                                      const std::string& pop,
                                      const std::string& method,
                                      const bbp::sonata::Selection& selection)
{
    const auto population = config.getNodePopulation(pop);
    const auto& attributes = population.attributeNames();
    return population.getAttribute<std::string>(method, selection);
}

} // namespace

void NeuronMaterialMap::setColor(brayns::ModelDescriptor* model, const brayns::Vector3f& color)
{
    _updateMaterial(model, soma, color);
    _updateMaterial(model, axon, color);
    _updateMaterial(model, dendrite, color);
    _updateMaterial(model, apicalDendrite, color);
}

NeuronColorHandler::NeuronColorHandler(brayns::ModelDescriptor* model,
                                       const std::string& configPath,
                                       const std::string& population)
 : PopulationColorHandler(model, configPath, population)
{
}

void NeuronColorHandler::_setElementsImpl(const std::vector<uint64_t>& ids,
                                          std::vector<ElementMaterialMap::Ptr>&& elements)
{
    _ids = ids;
    _elements = std::move(elements);
}

std::vector<std::string> NeuronColorHandler::_getMethodsImpl()
{
    const auto population = _config.getNodePopulation(_population);
    const auto& attributes = population.attributeNames();
    const std::vector<std::string> possibleMethods = {methodByMorphology, methodByLayer,
                                                      methodByMorphClass, methodByEType,
                                                      methodByMType, methodBySynapseClass,
                                                      methodByRegion, methodByHemisphere};
    std::vector<std::string> result;
    result.reserve(possibleMethods.size() + 1);
    result.push_back(methodBySection);

    for(const auto& possible : possibleMethods)
    {
        if(attributes.find(possible) != attributes.end())
            result.push_back(possible);
    }

    result.shrink_to_fit();
    return result;
}

std::vector<std::string> NeuronColorHandler::_getMethodVariablesImpl(const std::string& method)
{
    if(method == methodBySection)
        return {sectionSoma, sectionAxon, sectionDendrite, sectionApicalDendrite};

    const auto selection = bbp::sonata::Selection::fromValues(_ids);
    const auto values = ::getVariables(_config, _population, method, selection);
    const auto unique = std::unordered_set<std::string>(values.begin(), values.end());
    return std::vector<std::string>(unique.begin(), unique.end());
}

void NeuronColorHandler::_updateColorByIdImpl(const std::map<uint64_t, brayns::Vector3f>& colorMap)
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

void NeuronColorHandler::_updateSingleColorImpl(const brayns::Vector3f& color)
{
    for(auto& element : _elements)
        element->setColor(_model, color);
}

void
NeuronColorHandler::_updateColorImpl(const std::string& method, const ColorVariables& variables)
{
    if(!variables.empty())
        _colorWithInput(method, variables);
    else
        _colorRandomly(method);
}

void NeuronColorHandler::_colorWithInput(const std::string& method, const ColorVariables& input)
{
    if(method == methodBySection)
    {
        const auto updateSectionCB = [&](const char* section, size_t NeuronMaterialMap::*ptr)
        {
            const auto varIt = input.find(section);
            if(varIt == input.end())
                return;

            for(auto& element : _elements)
            {
                auto& nmm = static_cast<NeuronMaterialMap&>(*element.get());
                if(nmm.*ptr != std::numeric_limits<size_t>::max())
                    _updateMaterial(nmm.*ptr, varIt->second);
            }
        };
        updateSectionCB(sectionSoma, &NeuronMaterialMap::soma);
        updateSectionCB(sectionAxon, &NeuronMaterialMap::axon);
        updateSectionCB(sectionDendrite, &NeuronMaterialMap::dendrite);
        updateSectionCB(sectionApicalDendrite, &NeuronMaterialMap::apicalDendrite);
    }
    else
    {
        const auto selection = bbp::sonata::Selection::fromValues(_ids);
        const auto values = ::getVariables(_config, _population, method, selection);

        std::unordered_map<std::string, std::vector<size_t>> map;
        for(size_t i = 0; i < values.size(); ++i)
            map[values[i]].push_back(i);

        for(const auto& entry : input)
        {
            const auto& color = entry.second;
            auto it = map.find(entry.first);
            if(it != map.end())
            {
                for(const auto index : it->second)
                    _elements[index]->setColor(_model, color);
            }
        }
    }
}

void NeuronColorHandler::_colorRandomly(const std::string& method)
{
    if(method == methodBySection)
    {
        const auto updateSectionCB = [&](const brayns::Vector3f& c, size_t NeuronMaterialMap::*ptr)
        {
            for(auto& element : _elements)
            {
                auto& nmm = static_cast<NeuronMaterialMap&>(*element.get());
                if(nmm.*ptr != std::numeric_limits<size_t>::max())
                    _updateMaterial(nmm.*ptr, c);
            }
        };
        ColorRoulette r;
        updateSectionCB(r.getNextColor(), &NeuronMaterialMap::soma);
        updateSectionCB(r.getNextColor(), &NeuronMaterialMap::axon);
        updateSectionCB(r.getNextColor(), &NeuronMaterialMap::dendrite);
        updateSectionCB(r.getNextColor(), &NeuronMaterialMap::apicalDendrite);
    }
    else
    {
        const auto selection = bbp::sonata::Selection::fromValues(_ids);
        const auto values = ::getVariables(_config, _population, method, selection);

        ColorDeck deck;
        for(size_t i = 0; i < _elements.size(); ++i)
            _elements[i]->setColor(_model, deck.getColorForKey(values[i]));
    }
}
