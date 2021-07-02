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

#include "SonataCells.h"

#include <bbp/sonata/node_sets.h>

#include <glm/gtx/matrix_decompose.hpp>

namespace sonata
{
namespace data
{
namespace
{
constexpr char attribX[]               = "x";
constexpr char attribY[]               = "y";
constexpr char attribZ[]               = "z";
constexpr char attribOrientationX[]    = "orientation_x";
constexpr char attribOrientationY[]    = "orientation_y";
constexpr char attribOrientationZ[]    = "orientation_z";
constexpr char attribOrientationW[]    = "orientation_w";
constexpr char attribLayer[]           = "layer";
constexpr char attribRegion[]          = "region";
constexpr char attribMtype[]           = "mtype";
constexpr char attribEtype[]           = "etype";
constexpr char attribMorphology[]      = "morphology";
constexpr char attribVascStartX[]      = "start_x";
constexpr char attribVascStartY[]      = "start_y";
constexpr char attribVascStartZ[]      = "start_z";
constexpr char attribVascEndX[]        = "end_x";
constexpr char attribVascEndY[]        = "end_y";
constexpr char attribVascEndZ[]        = "end_z";
constexpr char attribVascStartD[]      = "start_diameter";
constexpr char attribVascEndD[]        = "end_diameter";
constexpr char attribVascStartNode[]   = "start_node";
constexpr char attribVascEndNode[]     = "end_node";
constexpr char attribVascSectionId[]   = "section_id";
constexpr char attribVascSegmentId[]   = "segment_id";

inline std::vector<std::string> getEnumValueList(const bbp::sonata::NodePopulation& population,
                                                 const bbp::sonata::Selection& selection,
                                                 const std::string& attribute)
{
    const auto enumValues = population.enumerationValues(attribute);
    const auto enumIndices = population.getEnumeration<size_t>(attribute, selection);

    std::vector<std::string> result(enumIndices.size());
    for(size_t i = 0; i < result.size(); ++i)
    {
        result[i] = enumValues[enumIndices[i]];
    }
    return result;
}

inline void checkValidType(const bbp::sonata::NodePopulation& nodes,
                           const std::vector<const char*>& validTypes)
{
   // nodes.getAttribute
}

inline void checkAttributes(const bbp::sonata::NodePopulation& nodes,
                            const std::vector<const char*>& attribs)
{
    const auto& attributes = nodes.attributeNames();
    for(const auto attrib : attribs)
    {
        if(attributes.find(attrib) == attributes.end())
        {
            throw std::runtime_error("Node population '" + nodes.name() + "' is missing "
                                     "attribute " + attrib);
        }
    }
}

}


std::vector<brayns::Vector3f> SonataCells::getPositions(const Nodes& nodes,
                                                        const Selection& selection)
{
    checkAttributes(nodes, {attribX, attribY, attribZ});

    const auto xPos = nodes.getAttribute<float>(attribX, selection);
    const auto yPos = nodes.getAttribute<float>(attribY, selection);
    const auto zPos = nodes.getAttribute<float>(attribZ, selection);

    std::vector<brayns::Vector3f> result (xPos.size());
    #pragma omp parallel for
    for(size_t i = 0; i < xPos.size(); ++i)
    {
        result[i].x = xPos[i];
        result[i].y = yPos[i];
        result[i].z = zPos[i];
    }
    return result;
}


std::vector<brayns::Quaternion> SonataCells::getRotations(const Nodes& nodes,
                                                          const Selection& selection)
{
    checkAttributes(nodes, {attribOrientationW,
                            attribOrientationX,
                            attribOrientationY,
                            attribOrientationZ});

    const auto x = nodes.getAttribute<float>(attribOrientationX, selection);
    const auto y = nodes.getAttribute<float>(attribOrientationY, selection);
    const auto z = nodes.getAttribute<float>(attribOrientationZ, selection);
    const auto w = nodes.getAttribute<float>(attribOrientationW, selection);

    std::vector<glm::quat> result (x.size());
    #pragma omp parallel for
    for(size_t i = 0; i < x.size(); ++i)
    {
        result[i].w = w[i];
        result[i].x = x[i];
        result[i].y = y[i];
        result[i].z = z[i];
    }
    return result;
}

std::vector<std::string> SonataCells::getLayers(const Nodes& nodes, const Selection& selection)
{
    checkAttributes(nodes, {attribLayer});
    return nodes.getAttribute<std::string>(attribLayer, selection);
}

std::vector<std::string> SonataCells::getRegions(const Nodes& nodes, const Selection& selection)
{
    checkAttributes(nodes, {attribRegion});
    return getEnumValueList(nodes, selection, attribRegion);
}

std::vector<std::string> SonataCells::getMTypes(const Nodes& nodes, const Selection& selection)
{
    checkAttributes(nodes, {attribMtype});
    return getEnumValueList(nodes, selection, attribMtype);
}

std::vector<std::string> SonataCells::getETypes(const Nodes& nodes, const Selection& selection)
{
    checkAttributes(nodes, {attribEtype});
    return getEnumValueList(nodes, selection, attribEtype);
}

std::vector<brayns::Vector3f>
SonataCells::getVasculatureStartPositions(const Nodes& nodes, const Selection& selection)
{
    checkAttributes(nodes, {attribVascStartX, attribVascStartY, attribVascStartZ});

    const auto xPos = nodes.getAttribute<float>(attribVascStartX, selection);
    const auto yPos = nodes.getAttribute<float>(attribVascStartY, selection);
    const auto zPos = nodes.getAttribute<float>(attribVascStartZ, selection);

    std::vector<brayns::Vector3f> result (xPos.size());
    #pragma omp parallel for
    for(size_t i = 0; i < xPos.size(); ++i)
    {
        result[i].x = xPos[i];
        result[i].y = yPos[i];
        result[i].z = zPos[i];
    }
    return result;
}

std::vector<brayns::Vector3f>
SonataCells::getVasculatureEndPositions(const Nodes& nodes, const Selection& selection)
{
    checkAttributes(nodes, {attribVascEndX, attribVascEndY, attribVascEndZ});

    const auto xPos = nodes.getAttribute<float>(attribVascEndX, selection);
    const auto yPos = nodes.getAttribute<float>(attribVascEndY, selection);
    const auto zPos = nodes.getAttribute<float>(attribVascEndZ, selection);

    std::vector<brayns::Vector3f> result (xPos.size());
    #pragma omp parallel for
    for(size_t i = 0; i < xPos.size(); ++i)
    {
        result[i].x = xPos[i];
        result[i].y = yPos[i];
        result[i].z = zPos[i];
    }
    return result;
}

std::vector<float>
SonataCells::getVasculatureStartDiameters(const Nodes& nodes, const Selection& selection)
{
    checkAttributes(nodes, {attribVascStartD});
    return nodes.getAttribute<float>(attribVascStartD, selection);
}

std::vector<float>
SonataCells::getVasculatureEndDiameters(const Nodes& nodes, const Selection& selection)
{
    checkAttributes(nodes, {attribVascEndD});
    return nodes.getAttribute<float>(attribVascEndD, selection);
}

std::vector<uint64_t>
SonataCells::getVasculatureStartingNodes(const Nodes& nodes, const Selection& selection)
{
    checkAttributes(nodes, {attribVascStartNode});
    return nodes.getAttribute<uint64_t>(attribVascStartNode, selection);
}

std::vector<uint64_t>
SonataCells::getVasculatureEndingNodes(const Nodes& nodes, const Selection& selection)
{
    checkAttributes(nodes, {attribVascEndNode});
    return nodes.getAttribute<uint64_t>(attribVascEndNode, selection);
}

std::vector<uint32_t>
SonataCells::getVasculatureSectionIds(const Nodes& nodes, const Selection& selection)
{
    checkAttributes(nodes, {attribVascSectionId});
    return nodes.getAttribute<uint32_t>(attribVascStartNode, selection);
}

std::vector<uint32_t>
SonataCells::getVasculatureSegmentIds(const Nodes& nodes, const Selection& selection)
{
    checkAttributes(nodes, {attribVascSegmentId});
    return nodes.getAttribute<uint32_t>(attribVascSegmentId, selection);
}
} // namespace data
} // namespace sonata
