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
#include <plugin/io/sonata/morphology/vasculature/VasculatureInstance.h>
#include <plugin/io/sonata/populations/nodes/colorhandlers/VasculatureColorHandler.h>

#include <set>

std::vector<MorphologyInstancePtr>
VasculaturePopulationLoader::load(const PopulationLoadConfig& loadSettings,
                                  const bbp::sonata::Selection& selection,
                                  const brayns::LoaderProgress& updateCb) const
{
    const auto startPoints = SonataVasculature::getSegmentStartPoints(_population, selection);
    const auto startRadii = SonataVasculature::getSegmentStartRadii(_population, selection);
    const auto endPoints = SonataVasculature::getSegmentEndPoints(_population, selection);
    const auto endRadii = SonataVasculature::getSegmentEndRadii(_population, selection);
    const auto sectionTypes = SonataVasculature::getSegmentSectionTypes(_population, selection);

    std::vector<MorphologyInstancePtr> result (startPoints.size());
    const auto radMult = loadSettings.vasculature.radiusMultiplier;

    constexpr VasculatureSection allSections[] =
    {
        VasculatureSection::ARTERIAL_CAPILLARY,
        VasculatureSection::ARTERIOLE,
        VasculatureSection::ARTERY,
        VasculatureSection::TRANSITIONAL,
        VasculatureSection::VEIN,
        VasculatureSection::VENOUS_CAPILLARY,
        VasculatureSection::VENULE
    };

    const auto& requestedSections = loadSettings.vasculature.sections;

    bool loadAll = true;
    for(const auto section : allSections)
    {
        if(requestedSections.find(section) == requestedSections.end())
        {
            loadAll = false;
            break;
        }
    }

    for(size_t i = 0; i < startPoints.size(); ++i)
    {
        if(!loadAll && requestedSections.find(sectionTypes[i]) == requestedSections.end())
            continue;

        result[i] = std::make_unique<VasculatureInstance>(startPoints[i],
                                                          startRadii[i] * radMult,
                                                          endPoints[i],
                                                          endRadii[i] * radMult,
                                                          sectionTypes[i]);
    }

    return result;
}

std::unique_ptr<CircuitColorHandler>
VasculaturePopulationLoader::createColorHandler(brayns::ModelDescriptor *model,
                                                const std::string& config) const noexcept
{
    return std::make_unique<VasculatureColorHandler>(model, config, _population.name());
}
