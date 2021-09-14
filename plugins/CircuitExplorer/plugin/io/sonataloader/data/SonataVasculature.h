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

#include <bbp/sonata/nodes.h>

#include <brayns/common/mathTypes.h>

#include <plugin/io/morphology/vasculature/VasculatureSection.h>

namespace sonataloader
{
class SonataVasculature
{
public:
    using Nodes = bbp::sonata::NodePopulation;
    using Selection = bbp::sonata::Selection;

    static std::vector<brayns::Vector3f>
    getSegmentStartPoints(const Nodes& nodes, const Selection& selection);

    static std::vector<brayns::Vector3f>
    getSegmentEndPoints(const Nodes& nodes, const Selection& selection);

    static std::vector<float>
    getSegmentStartRadii(const Nodes& nodes, const Selection& selection);

    static std::vector<float>
    getSegmentEndRadii(const Nodes& nodes, const Selection& selection);

    static std::vector<uint32_t>
    getSegmentSectionIds(const Nodes& nodes, const Selection& selection);

    static std::vector<bbp::sonata::NodeID>
    getSegmentStartNodes(const Nodes& nodes, const Selection& selection);

    static std::vector<bbp::sonata::NodeID>
    getSegmentEndNodes(const Nodes& nodes, const Selection& selection);

    static std::vector<VasculatureSection>
    getSegmentSectionTypes(const Nodes& nodes, const Selection& selection);
};
}
