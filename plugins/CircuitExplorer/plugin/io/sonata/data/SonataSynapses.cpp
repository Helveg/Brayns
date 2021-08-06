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

namespace
{
constexpr char attribEffSectionId[]     = "efferent_section_id";
constexpr char attribEffSegmentId[]     = "efferent_segment_id";
constexpr char attribEffSegmentPos[]    = "efferent_section_pos";
constexpr char attribEffSurfPosiX[]     = "efferent_surface_x";
constexpr char attribEffSurfPosiY[]     = "efferent_surface_y";
constexpr char attribEffSurfPosiZ[]     = "efferent_surface_z";

constexpr char attribAffSectionId[]     = "afferent_section_id";
constexpr char attribAffSegmentId[]     = "afferent_segment_id";
constexpr char attribAffSegmentPos[]    = "afferent_section_pos";
constexpr char attribAffSurfPosiX[]     = "afferent_surface_x";
constexpr char attribAffSurfPosiY[]     = "afferent_surface_y";
constexpr char attribAffSurfPosiZ[]     = "afferent_surface_z";

void checkEdgeParameters(const bbp::sonata::EdgePopulation& population,
                         const std::vector<const char*>& inputAttribs)
{
    const auto& attribs = population.attributeNames();
    for(const auto& attribute : inputAttribs)
    {
        if(attribs.find(attribute) == attribs.end())
            throw std::runtime_error("Edge population " + population.name()
                                     + " is missing attribute " + attribute);
    }
}

inline std::vector<brayns::Vector3f> loadSurfacePos(const bbp::sonata::EdgePopulation& population,
                                                    const bbp::sonata::Selection& selection,
                                                    const char* attribX,
                                                    const char* attribY,
                                                    const char* attribZ)
{
    checkEdgeParameters(population, {attribX, attribY, attribZ});
    const auto surfaPosXs = population.getAttribute<float>(attribX, selection);
    const auto surfaPosYs = population.getAttribute<float>(attribY, selection);
    const auto surfaPosZs = population.getAttribute<float>(attribZ, selection);

    if(surfaPosXs.size() != surfaPosYs.size() || surfaPosXs.size() != surfaPosZs.size())
        throw std::runtime_error("Edge population '" + population.name() + "' surface position "
                                 "parameters missmatch in size");

    std::vector<brayns::Vector3f> result (surfaPosXs.size(), brayns::Vector3f(0.f, 0.f, 0.f));
    for(size_t i = 0; i < surfaPosXs.size(); ++i)
    {
        result[i].x = surfaPosXs[i];
        result[i].y = surfaPosYs[i];
        result[i].z = surfaPosZs[i];
    }
    return result;
}

inline void fixSections(std::vector<int32_t>& sectionIds)
{
    std::transform(sectionIds.begin(), sectionIds.end(), sectionIds.begin(), [](int32_t secId)
    {
        return secId - 1;
    });
}
}


std::vector<uint64_t> SonataSynapses::getAfferentSourceNodes(const Edges& population,
                                                             const Selection& selection)
{
    return population.sourceNodeIDs(selection);
}

std::vector<uint64_t> SonataSynapses::getAfferentTargetNodes(const Edges& population,
                                                             const Selection& selection)
{
    return population.targetNodeIDs(selection);
}

std::vector<uint64_t> SonataSynapses::getEfferentSourceNodes(const Edges& population,
                                                             const Selection& selection)
{
    return population.sourceNodeIDs(selection);
}

std::vector<uint64_t> SonataSynapses::getEfferentTargetNodes(const Edges& population,
                                                             const Selection& selection)
{
    return population.targetNodeIDs(selection);
}

std::vector<int32_t> SonataSynapses::getAfferentSectionIds(const Edges& population,
                                                           const Selection& selection)
{
    checkEdgeParameters(population, {attribAffSectionId});
    auto sectionIds = population.getAttribute<int32_t>(attribAffSectionId, selection);
    fixSections(sectionIds);
    return sectionIds;
}

std::vector<int32_t> SonataSynapses::getEfferentSectionIds(const Edges& population,
                                                           const Selection& selection)
{
    checkEdgeParameters(population, {attribEffSectionId});
    auto sectionIds = population.getAttribute<int32_t>(attribEffSectionId, selection);
    fixSections(sectionIds);
    return sectionIds;
}

std::vector<brayns::Vector3f> SonataSynapses::getAfferentSurfacePos(const Edges& population,
                                                                    const Selection& selection)
{
    return loadSurfacePos(population,
                          selection,
                          attribAffSurfPosiX,
                          attribAffSurfPosiY,
                          attribAffSurfPosiZ);
}

std::vector<brayns::Vector3f> SonataSynapses::getEfferentSurfacePos(const Edges& population,
                                                                    const Selection& selection)
{
    return loadSurfacePos(population,
                          selection,
                          attribEffSurfPosiX,
                          attribEffSurfPosiY,
                          attribEffSurfPosiZ);
}

std::vector<float> SonataSynapses::getAfferentSectionDistances(const Edges& population,
                                                               const Selection& selection)
{
    checkEdgeParameters(population, {attribAffSegmentPos});
    return population.getAttribute<float>(attribAffSegmentPos, selection);
}

std::vector<float> SonataSynapses::getEfferentSectionDistances(const Edges& population,
                                                               const Selection& selection)
{
    checkEdgeParameters(population, {attribEffSegmentPos});
    return population.getAttribute<float>(attribEffSegmentPos, selection);
}
