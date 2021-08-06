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

#include <bbp/sonata/nodes.h>

#include <brayns/common/mathTypes.h>


/**
 * @brief The SonataCells class is in charge of quering cell information from the
 *        node network files
 */
class SonataCells
{
public:
    using Nodes = bbp::sonata::NodePopulation;
    using Selection = bbp::sonata::Selection;

    static
    std::vector<std::string> getMorphologies(const Nodes& nodes,
                                             const Selection& selection);

    static
    std::vector<brayns::Vector3f> getPositions(const Nodes& nodes,
                                               const Selection& selection);

    static
    std::vector<brayns::Quaternion> getRotations(const Nodes& nodes,
                                                 const Selection& selection);

    static
    std::vector<std::string> getLayers(const Nodes& population,
                                       const Selection& selection);

    static
    std::vector<std::string> getRegions(const Nodes& population,
                                        const Selection& selection);

    static
    std::vector<std::string> getMTypes(const Nodes& population,
                                       const Selection& selection);

    static
    std::vector<std::string> getETypes(const Nodes& population,
                                       const Selection& selection);

    static
    std::vector<brayns::Vector3f> getVasculatureStartPositions(const Nodes& nodes,
                                                               const Selection& selection);

    static
    std::vector<brayns::Vector3f> getVasculatureEndPositions(const Nodes& nodes,
                                                             const Selection& selection);

    static
    std::vector<float> getVasculatureStartDiameters(const Nodes& nodes,
                                                    const Selection& selection);

    static
    std::vector<float> getVasculatureEndDiameters(const Nodes& nodes,
                                                  const Selection& selection);

    static
    std::vector<uint64_t> getVasculatureStartingNodes(const Nodes& nodes,
                                                      const Selection& selection);

    static
    std::vector<uint64_t> getVasculatureEndingNodes(const Nodes& nodes,
                                                    const Selection& selection);

    static
    std::vector<uint32_t> getVasculatureSectionIds(const Nodes& nodes,
                                                   const Selection& selection);

    static
    std::vector<uint32_t> getVasculatureSegmentIds(const Nodes& nodes,
                                                   const Selection& selection);
};
