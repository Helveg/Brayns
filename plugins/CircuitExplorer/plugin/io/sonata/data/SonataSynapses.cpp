/* Copyright (c) 2015-2021, EPFL/Blue Brain Project
 * All rights reserved. Do not distribute without permission.
 * Responsible Author: Nadir Roman <nadir.romanguerrero@epfl.ch>
 *
 * This file is part of the circuit explorer for Brayns
 * <https://github.com/favreau/Brayns-UC-CircuitExplorer>
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

#include "SonataSynapses.h"

#include <algorithm>
#include <map>
#include <random>

namespace sonata
{
namespace data
{
namespace
{
constexpr char attribEffSectionId[] = "efferent_section_id";
constexpr char attribEffSegmentId[] = "efferent_segment_id";
constexpr char attribEffSurfPosiX[] = "efferent_surface_x";
constexpr char attribEffSurfPosiY[] = "efferent_surface_y";
constexpr char attribEffSurfPosiZ[] = "efferent_surface_z";

constexpr char attribAffSectionId[] = "afferent_section_id";
constexpr char attribAffSegmentId[] = "afferent_segment_id";
constexpr char attribAffSurfPosiX[] = "afferent_surface_x";
constexpr char attribAffSurfPosiY[] = "afferent_surface_y";
constexpr char attribAffSurfPosiZ[] = "afferent_surface_z";

bbp::sonata::Selection computePercentage(const bbp::sonata::Selection& src, const float percent)
{
    auto commonList = src.flatten();

    std::random_device randomDevice;
    std::mt19937_64 randomEngine(randomDevice());
    std::shuffle(commonList.begin(), commonList.end(), randomEngine);
    const size_t finalSize = static_cast<size_t>(
                static_cast<double>(percent) * static_cast<double>(commonList.size()));
    commonList.resize(finalSize);
    std::sort(commonList.begin(), commonList.end());

    return bbp::sonata::Selection::fromValues(commonList);
}

}

// post synaptic surface position
std::vector<std::vector<Synapse>>
SonataSynapses::getAfferent(const bbp::sonata::EdgePopulation& population,
                            const bbp::sonata::Selection& selection,
                            const float percentage)
{
    return loadSynapses(attribAffSectionId,
                        attribAffSegmentId,
                        attribAffSurfPosiX,
                        attribAffSurfPosiY,
                        attribAffSurfPosiZ,
                        selection,
                        population,
                        percentage,
                        true);
}

// pre synaptic surface position
std::vector<std::vector<Synapse>>
SonataSynapses::getEfferent(const bbp::sonata::EdgePopulation& population,
                            const bbp::sonata::Selection& selection,
                            const float percentage)
{
    return loadSynapses(attribEffSectionId,
                        attribEffSegmentId,
                        attribEffSurfPosiX,
                        attribEffSurfPosiY,
                        attribEffSurfPosiZ,
                        selection,
                        population,
                        percentage,
                        false);
}

std::vector<std::vector<Synapse>>
SonataSynapses::loadSynapses(const char* sectionAttrib,
                             const char* segmentAttrib,
                             const char* posXAttrib,
                             const char* posYAttrib,
                             const char* posZAttrib,
                             const bbp::sonata::Selection& selection,
                             const bbp::sonata::EdgePopulation& population,
                             const float percentage,
                             const bool afferent)
{
    const std::vector<const char*> attribCheck =
    {
        sectionAttrib,
        segmentAttrib,
        posXAttrib,
        posYAttrib,
        posZAttrib
    };
    const auto& attribs = population.attributeNames();
    for(const auto& attribute : attribCheck)
    {
        if(attribs.find(attribute) == attribs.end())
            throw std::runtime_error("Edge population " + population.name()
                                     + " is missing attribute " + attribute);
    }

    const auto sourceNodeIds = selection.flatten();
    // Use an ordered map to map each node Id to its synapses, so that in the end
    // we can return a vector of vectors ordered by ascending node Id, which we will
    // use to access cell specific synapses in O(1) by using its index
    // (Cell index represents its position on a vector based on its ordered node Id value)
    std::map<uint64_t, std::vector<Synapse>> mapping;
    for(size_t i = 0; i < sourceNodeIds.size(); ++i)
        mapping[sourceNodeIds[i]] = std::vector<Synapse>();


    const bbp::sonata::Selection baseSelection = afferent? population.afferentEdges(sourceNodeIds)
                                                         : population.efferentEdges(sourceNodeIds);
    const bbp::sonata::Selection edgeSelection = computePercentage(baseSelection, percentage);

    const auto flattenEdges = edgeSelection.flatten();
    auto preNodeIds =  population.sourceNodeIDs(edgeSelection);
    auto postNodeIds = population.targetNodeIDs(edgeSelection);

    if(afferent)
        preNodeIds.swap(postNodeIds);

    const auto sectionIds = population.getAttribute<int32_t>(sectionAttrib, edgeSelection);
    const auto surfaPosXs = population.getAttribute<float>(posXAttrib, edgeSelection);
    const auto surfaPosYs = population.getAttribute<float>(posYAttrib, edgeSelection);
    const auto surfaPosZs = population.getAttribute<float>(posZAttrib, edgeSelection);

    const auto size = flattenEdges.size();

    if(sectionIds.size() != size || surfaPosXs.size() != size
            || surfaPosYs.size() != size || surfaPosZs.size() != size)
        throw std::runtime_error("Edge population " + population.name() + " attributes size "
                                 "does have different sizes for the same selection");

    for(size_t i = 0; i < size; ++i)
    {
        auto& cellSynapseBuffer = mapping[preNodeIds[i]];
        cellSynapseBuffer.push_back(Synapse(sectionIds[i],
                                            flattenEdges[i],
                                            brayns::Vector3f(surfaPosXs[i],
                                                             surfaPosYs[i],
                                                             surfaPosZs[i])));
    }

    std::vector<std::vector<Synapse>> result (sourceNodeIds.size());
    size_t i = 0;
    for(auto& entry : mapping)
        result[i++] = std::move(entry.second);
    return result;
}


std::vector<uint64_t> SonataSynapses::getAfferentSourceNodes(const Edges& population,
                                                             const Selection& selection)
{

}

std::vector<uint64_t> SonataSynapses::getAfferentTargetNodes(const Edges& population,
                                                             const Selection& selection)
{

}

std::vector<uint64_t> SonataSynapses::getEfferentSourceNodes(const Edges& population,
                                                             const Selection& selection)
{

}

std::vector<uint64_t> SonataSynapses::getEfferentTargetNodes(const Edges& population,
                                                             const Selection& selection)
{

}


std::vector<int32_t> SonataSynapses::getSectionIds(const Edges& population,
                                                   const Selection& selection)
{

}


std::vector<brayns::Vector3f> SonataSynapses::getSurfacePos(const Edges& population,
                                                            const Selection& selection)
{

}

} // namespace data
} // namespace sonata
