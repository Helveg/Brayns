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

#include "VasculaturePopulationLoader.h"

#include <plugin/io/sonata/data/SonataVasculature.h>
#include <plugin/io/sonata/morphology/vasculature/VasculatureMorphology.h>
#include <plugin/io/sonata/morphology/vasculature/VasculatureSDFBuilder.h>

std::vector<MorphologyInstancePtr>
VasculaturePopulationLoader::load(const PopulationLoadConfig& loadSettings,
                                  const bbp::sonata::Selection& selection,
                                  const brayns::LoaderProgress& updateCb) const
{
    const auto startPoints = SonataVasculature::getSegmentStartPoints(_population, selection);
    const auto startRadii = SonataVasculature::getSegmentStartRadii(_population, selection);
    const auto endPoints = SonataVasculature::getSegmentEndPoints(_population, selection);
    const auto endRadii = SonataVasculature::getSegmentEndRadii(_population, selection);
    const auto sectionIds = SonataVasculature::getSegmentSectionIds(_population, selection);
    const auto sectionTypes = SonataVasculature::getSegmentSectionTypes(_population, selection);
    const auto startNodes = SonataVasculature::getSegmentStartNodes(_population, selection);

    uint32_t highestSection = 0;
    for(const auto sectionId : sectionIds)
        highestSection = sectionId > highestSection? sectionId : highestSection;

    VasculatureMorphology morphology;
    auto& sections = morphology.sections();
    sections.resize(highestSection + 1);

    for(size_t i = 0; i < sectionIds.size(); ++i)
    {
        sections[i].id = sectionIds[i];
        sections[i].parentId = sectionIds[startNodes[i]];
        sections[i].type = sectionTypes[i];
        sections[i].segments.push_back({startPoints[i], startRadii[i], endPoints[i], endRadii[i]});
    }

    auto morphologyInstance = VasculatureSDFBuilder().build(morphology);

    std::vector<MorphologyInstancePtr> result;
    result.push_back(std::move(morphologyInstance));
    return result;
}
