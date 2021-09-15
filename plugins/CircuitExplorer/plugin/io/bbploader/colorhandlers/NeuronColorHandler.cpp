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

#include <brain/brain.h>
#include <brion/brion.h>

#include <mvdtool/mvd3.hpp>
#include <mvdtool/sonata.hpp>

#include <brayns/common/utils/stringUtils.h>

#include <plugin/api/ColorUtils.h>
#include <plugin/api/Log.h>
#include <plugin/io/morphology/neuron/NeuronMaterialMap.h>
#include <plugin/io/morphology/neuron/NeuronSection.h>

namespace bbploader
{
namespace
{
constexpr char methodBySection[] = "morphology section";
constexpr char methodByLayer[]   = "layer";
constexpr char methodByEType[]   = "etype";
constexpr char methodByMtype[]   = "mtype";
constexpr char methodByMorph[]   = "morphology name";

// Custom classes to wrap around the different circuit formats with a common interface
class CircuitAccessor
{
public:
    virtual std::vector<std::string> getLayers(const brain::GIDSet& gids) const = 0;
    virtual std::vector<std::string> getETypes(const brain::GIDSet& gids) const = 0;
    virtual std::vector<std::string> getMTypes(const brain::GIDSet& gids) const = 0;
    virtual std::vector<std::string> getMorphologyNames(const brain::GIDSet& gids) const = 0;

    std::vector<std::string> getData(const brain::GIDSet& gids, const std::string& method) const
    {
        if(method == methodByLayer)
            return getLayers(gids);
        else if(method == methodByEType)
            return getETypes(gids);
        else if(method == methodByMtype)
            return getMTypes(gids);
        else if(method == methodByMorph)
            return getMorphologyNames(gids);
        return {};
    }
};

class MVD2Circuit : public CircuitAccessor
{
public:
    MVD2Circuit(const std::string& path)
     : _circuit(path)
    {
    }

    std::vector<std::string> getLayers(const brain::GIDSet& gids) const final
    {
        const auto result = _getAttrib(gids, brion::NeuronAttributes::NEURON_LAYER);
        if(result.empty())
            PLUGIN_WARN << "MVD2Circuit: No layer data found" << std::endl;
        return result;
    }

    std::vector<std::string> getETypes(const brain::GIDSet& gids) const final
    {
        const auto indices = _getAttrib(gids, brion::NeuronAttributes::NEURON_ETYPE);
        if(indices.empty())
        {
            PLUGIN_WARN << "MVD2Circuit: No e-type data found" << std::endl;
            return {};
        }
        const auto allETypes = _circuit.getTypes(brion::NeuronClass::NEURONCLASS_ETYPE);
        std::vector<std::string> result (gids.size());
        for(size_t i = 0; i < gids.size(); ++i)
            result[i] = allETypes[std::stoull(indices[i])];
        return result;
    }

    std::vector<std::string> getMTypes(const brain::GIDSet& gids) const final
    {
        const auto indices = _getAttrib(gids, brion::NeuronAttributes::NEURON_MTYPE);
        if(indices.empty())
        {
            PLUGIN_WARN << "MVD2Circuit: No m-type data found" << std::endl;
            return {};
        }
        const auto allMTypes = _circuit.getTypes(brion::NeuronClass::NEURONCLASS_MTYPE);
        std::vector<std::string> result (gids.size());
        for(size_t i = 0; i < gids.size(); ++i)
            result[i] = allMTypes[std::stoull(indices[i])];
        return result;
    }

    std::vector<std::string> getMorphologyNames(const brain::GIDSet& gids) const final
    {
        const auto result = _getAttrib(gids, brion::NeuronAttributes::NEURON_MORPHOLOGY_NAME);
        if(result.empty())
            PLUGIN_WARN << "MVD2Circuit: No morphology name data found" << std::endl;
        return result;
    }
private:
    std::vector<std::string> _getAttrib(const brain::GIDSet& gids, const uint32_t attrib) const
    {
        const auto matrix = _circuit.get(gids, attrib);
        if(matrix.shape()[0] == 0)
            return {};

        const size_t idx = matrix.shape()[1] > 1? 1 : 0;
        std::vector<std::string> data (gids.size());
        for(size_t i = 0; i < gids.size(); ++i)
            data[i] = matrix[i][idx];
        return data;
    }
    brion::Circuit _circuit;
};

template<class CircuitType>
class GenericCircuit : public CircuitAccessor
{
public:
    GenericCircuit(std::unique_ptr<CircuitType>&& circuit)
     : _circuit(std::move(circuit))
    {
    }

    std::vector<std::string> getLayers(const brain::GIDSet& gids) const final
    {
        const auto range = _getRange(gids);
        const auto result = _arrange(_circuit->getLayers(range), range, gids);
        if(result.empty())
            PLUGIN_WARN << "GenericCircuit: No layer data found" << std::endl;
        return result;
    }

    std::vector<std::string> getETypes(const brain::GIDSet& gids) const final
    {
        const auto range = _getRange(gids);
        const auto result = _arrange(_circuit->getEtypes(range), range, gids);
        if(result.empty())
            PLUGIN_WARN << "GenericCircuit: No e-type data found" << std::endl;
        return result;
    }

    std::vector<std::string> getMTypes(const brain::GIDSet& gids) const final
    {
        const auto range = _getRange(gids);
        const auto result = _arrange(_circuit->getMtypes(range), range, gids);
        if(result.empty())
            PLUGIN_WARN << "GenericCircuit: No m-type data found" << std::endl;
        return result;
    }

    std::vector<std::string> getMorphologyNames(const brain::GIDSet& gids) const final
    {
        const auto range = _getRange(gids);
        const auto result = _arrange(_circuit->getMorphologies(range), range, gids);
        if(result.empty())
            PLUGIN_WARN << "GenericCircuit: No morphology name data found" << std::endl;
        return result;
    }

private:
    MVD::Range _getRange(const brain::GIDSet& gids) const noexcept
    {
        MVD::Range range;
        range.offset = *gids.begin() - 1; // GIDs start at 1
        range.count = *gids.rbegin() - range.offset;
        return range;
    }

    std::vector<std::string> _arrange(const std::vector<std::string>& src,
                                      const MVD::Range& range,
                                      const brain::GIDSet& gids) const
    {
        if(src.empty())
            return {};
        std::vector<std::string> result (gids.size());
        auto it = gids.begin();
        auto prevGID = *it;
        size_t srcIdx = 0;
        for(size_t i = 0; i < result.size(); ++i, ++it)
        {
            const auto offset = *it - prevGID;
            srcIdx += offset;
            if(srcIdx > src.size())
                throw std::runtime_error("Vector overflowed access");
            result[i] = src[srcIdx];
        }
        return result;
    }

    std::unique_ptr<CircuitType> _circuit;
};

// ------------------------------------------------------------------------------------------------

inline std::unique_ptr<CircuitAccessor> __instantiateCircuit(const std::string& path,
                                                             const std::string& pop)
{
    const auto lowerCasePath = brayns::string_utils::toLowercase(path);
    if(lowerCasePath.find(".mvd2") != std::string::npos)
        return std::make_unique<MVD2Circuit>(path);
    else if(lowerCasePath.find(".mvd3") != std::string::npos)
        return std::make_unique<GenericCircuit<MVD3::MVD3File>>(
                                    std::make_unique<MVD3::MVD3File>(path));
    else if(lowerCasePath.find(".h5") || lowerCasePath.find(".hdf5"))
        return std::make_unique<GenericCircuit<MVD::SonataFile>>(
                                    std::make_unique<MVD::SonataFile>(path, pop));

    return {nullptr};
}

inline std::vector<std::string> __getAvailableMethods(const CircuitAccessor& circuit)
{
    std::vector<std::string> result;
    result.push_back(methodBySection);

    const auto layerData = circuit.getLayers({1});
    if(!layerData.empty() && !layerData[0].empty())
        result.push_back(methodByLayer);

    const auto mTypeData = circuit.getMTypes({1});
    if(!mTypeData.empty() && !mTypeData[0].empty())
        result.push_back(methodByMtype);

    const auto eTypeData = circuit.getETypes({1});
    if(!eTypeData.empty() && !eTypeData[0].empty())
        result.push_back(methodByEType);

    const auto morphData = circuit.getMorphologyNames({1});
    if(!morphData.empty() && !morphData[0].empty())
        result.push_back(methodByMorph);

    return result;
}
}

NeuronColorHandler::NeuronColorHandler(brayns::ModelDescriptor* model,
                                       const std::string& circuitPath,
                                       const std::string& circuitPop)
 : CircuitColorHandler(model)
 , _circuitPath(circuitPath)
 , _circuitPop(circuitPop)
{
}

void NeuronColorHandler::_setElementsImpl(const std::vector<uint64_t>& ids,
                                          std::vector<ElementMaterialMap::Ptr>&& elements)
{
    _gids = brain::GIDSet(ids.begin(), ids.end());
    _cells = std::move(elements);
}

std::vector<std::string> NeuronColorHandler::_getMethodsImpl() const
{
    const auto circuit = __instantiateCircuit(_circuitPath, _circuitPop);
    return __getAvailableMethods(*circuit);
}

std::vector<std::string>
NeuronColorHandler::_getMethodVariablesImpl(const std::string& method) const
{
    if(method == methodBySection)
        return EnumWrapper<NeuronSection>().toStringList();

    const auto circuit = __instantiateCircuit(_circuitPath, _circuitPop);
    std::vector<std::string> data;
    if(method == methodByLayer)
        data = circuit->getLayers(_gids);
    else if(method == methodByEType)
        data = circuit->getETypes(_gids);
    else if(method == methodByMtype)
        data = circuit->getMTypes(_gids);
    else if(method == methodByMorph)
        data = circuit->getMorphologyNames(_gids);

    const std::unordered_set<std::string> uniques (data.begin(), data.end());
    return std::vector<std::string>(uniques.begin(), uniques.end());
}

void NeuronColorHandler::_updateColorByIdImpl(const std::map<uint64_t, brayns::Vector4f>& colorMap)
{
    if(!colorMap.empty())
    {
        auto it = colorMap.begin();
        auto idIt = _gids.begin();
        size_t index = 0;
        while(it != colorMap.end() && idIt != _gids.end())
        {
            while(it->first != *idIt && idIt != _gids.end())
            {
                ++idIt;
                ++index;
            }
            if(index >= _cells.size())
                throw std::invalid_argument("Requested coloring GID '" + std::to_string(it->first)
                                            + "' is beyond the highest GID loaded '"
                                            + std::to_string(*_gids.rbegin()) + "'");

            _cells[index]->setColor(_model, it->second);
            ++it;
        }
    }
    else
    {
        ColorRoulette r;
        for(auto& element : _cells)
            element->setColor(_model, r.getNextColor());
    }
}

void NeuronColorHandler::_updateSingleColorImpl(const brayns::Vector4f& color)
{
    for(auto& element : _cells)
        element->setColor(_model, color);
}

void
NeuronColorHandler::_updateColorImpl(const std::string& method, const ColorVariables& input)
{
    if(!input.empty())
    {
        if(method == methodBySection)
        {
            const auto updateSectionCB = [&](const std::string& section,
                                             size_t NeuronMaterialMap::*ptr)
            {
                const auto varIt = input.find(section);
                if(varIt == input.end())
                    return;

                for(auto& element : _cells)
                {
                    auto& nmm = static_cast<NeuronMaterialMap&>(*element.get());
                    if(nmm.*ptr != std::numeric_limits<size_t>::max())
                        _updateMaterial(nmm.*ptr, varIt->second);
                }
            };
            const EnumWrapper<NeuronSection> ns;
            updateSectionCB(ns.toString(NeuronSection::SOMA), &NeuronMaterialMap::soma);
            updateSectionCB(ns.toString(NeuronSection::AXON), &NeuronMaterialMap::axon);
            updateSectionCB(ns.toString(NeuronSection::DENDRITE), &NeuronMaterialMap::dendrite);
            updateSectionCB(ns.toString(NeuronSection::APICAL_DENDRITE),
                            &NeuronMaterialMap::apicalDendrite);
        }
        else
        {
            const auto circuit = __instantiateCircuit(_circuitPath, _circuitPop);
            const auto values = circuit->getData(_gids, method);

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
                        _cells[index]->setColor(_model, color);
                }
            }
        }
    }
    else
    {
        if(method == methodBySection)
        {
            const auto updateSectionCB = [&](const brayns::Vector4f& c,
                                             size_t NeuronMaterialMap::*ptr)
            {
                for(auto& element : _cells)
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
            const auto circuit = __instantiateCircuit(_circuitPath, _circuitPop);
            const auto values = circuit->getData(_gids, method);

            ColorDeck deck;
            for(size_t i = 0; i < _cells.size(); ++i)
                _cells[i]->setColor(_model, deck.getColorForKey(values[i]));
        }
    }
}
}
