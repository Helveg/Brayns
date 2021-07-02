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

#pragma once

#include <bbp/sonata/edges.h>

#include <brayns/common/mathTypes.h>


namespace sonata
{
namespace data
{
class SonataSynapses
{
private:
    using Edges = bbp::sonata::EdgePopulation;
    using Selection = bbp::sonata::Selection;
public:
    static
    std::vector<uint64_t> getAfferentSourceNodes(const Edges& population,
                                                 const Selection& selection);
    static
    std::vector<uint64_t> getAfferentTargetNodes(const Edges& population,
                                                 const Selection& selection);

    static
    std::vector<uint64_t> getEfferentSourceNodes(const Edges& population,
                                                 const Selection& selection);
    static
    std::vector<uint64_t> getEfferentTargetNodes(const Edges& population,
                                                 const Selection& selection);

    static
    std::vector<int32_t> getSectionIds(const Edges& population, const Selection& selection);

    static
    std::vector<brayns::Vector3f> getSurfacePos(const Edges& population,
                                                const Selection& selection);

private:
    static std::vector<std::vector<Synapse>>
    loadSynapses(const char* sectionAttrib,
                 const char* segmentAttrib,
                 const char* posXAttrib,
                 const char* posYAttrib,
                 const char* posZAttrib,
                 const Selection& selection,
                 const Edges& population,
                 const float percentage,
                 const bool afferent);
};
} // namespace data
} // namespace sonata
