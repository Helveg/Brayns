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

#include <bbp/sonata/edges.h>

#include <brayns/common/mathTypes.h>


class SonataSynapses
{
private:
    using Edges = bbp::sonata::EdgePopulation;
    using Selection = bbp::sonata::Selection;
public:
    static
    std::vector<uint64_t> getSourceNodes(const Edges& population,
                                                 const Selection& edgeSelection);
    static
    std::vector<uint64_t> getTargetNodes(const Edges& population,
                                                 const Selection& edgeSelection);
    static
    std::vector<int32_t> getAfferentSectionIds(const Edges& population,
                                               const Selection& edgeSelection);

    static
    std::vector<int32_t> getEfferentSectionIds(const Edges& population,
                                               const Selection& edgeSelection);

    static
    std::vector<brayns::Vector3f> getAfferentSurfacePos(const Edges& population,
                                                        const Selection& selection);

    static
    std::vector<brayns::Vector3f> getEfferentSurfacePos(const Edges& population,
                                                        const Selection& selection);

    static
    std::vector<float> getAfferentSectionDistances(const Edges& population,
                                                   const Selection& selection);

    static
    std::vector<float> getEfferentSectionDistances(const Edges& population,
                                                   const Selection& selection);

    static
    std::vector<int32_t> getEfferentAstrocyteSectionIds(const Edges& population,
                                                        const Selection& selection);

    static
    std::vector<float> getEfferentAstrocyteSectionDistances(const Edges& population,
                                                            const Selection& selection);

    static std::vector<brayns::Vector3f>
    getEndFeetSurfacePos(const Edges& population, const Selection& selection);

    static std::vector<uint64_t>
    getEndFeetIds(const Edges& population, const Selection& selection);

};
