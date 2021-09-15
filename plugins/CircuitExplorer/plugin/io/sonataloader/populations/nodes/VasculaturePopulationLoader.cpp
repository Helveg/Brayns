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

#include <plugin/api/Log.h>
#include <plugin/io/morphology/vasculature/VasculatureInstance.h>
#include <plugin/io/sonataloader/data/SonataVasculature.h>
#include <plugin/io/sonataloader/populations/nodes/colorhandlers/VasculatureColorHandler.h>

namespace sonataloader
{
std::vector<MorphologyInstancePtr>
VasculaturePopulationLoader::load(const PopulationLoadConfig& loadSettings,
                                  const bbp::sonata::Selection& selection,
                                  SubProgressReport& cb) const
{
    const auto startPoints = SonataVasculature::getSegmentStartPoints(_population, selection); 
    const auto endPoints = SonataVasculature::getSegmentEndPoints(_population, selection);
    const auto sectionTypes = SonataVasculature::getSegmentSectionTypes(_population, selection);

    std::vector<float> startRadii, endRadii;
    if(loadSettings.neurons.radiusOverride > 0.f)
    {
        startRadii.resize(startPoints.size(), loadSettings.neurons.radiusOverride);
        endRadii.resize(endPoints.size(), loadSettings.neurons.radiusOverride);
    }
    else
    {
        startRadii = SonataVasculature::getSegmentStartRadii(_population, selection);
        endRadii = SonataVasculature::getSegmentEndRadii(_population, selection);
        if(loadSettings.neurons.radiusMultiplier != 1.f)
        {
            std::transform(startRadii.begin(),
                           startRadii.end(),
                           startRadii.begin(),
                           [mult = loadSettings.neurons.radiusMultiplier](const float r)
            {
                return r * mult;
            });
            std::transform(endRadii.begin(),
                           endRadii.end(),
                           endRadii.begin(),
                           [mult = loadSettings.neurons.radiusMultiplier](const float r)
            {
                return r * mult;
            });
        }
    }

    std::vector<MorphologyInstancePtr> result (startPoints.size());
    const auto requestedSections = loadSettings.vasculature.sections;

    PLUGIN_WARN << "Vasculature section check disabled. Test data has wrong 'type' dataset"
                << std::endl;

    for(size_t i = 0; i < startPoints.size(); ++i)
    {
        if(sectionTypes[i] != VasculatureSection::NONE
                && !static_cast<uint8_t>(sectionTypes[i] & requestedSections))
            continue;

        result[i] = std::make_unique<VasculatureInstance>(startPoints[i],
                                                          startRadii[i],
                                                          endPoints[i],
                                                          endRadii[i],
                                                          sectionTypes[i]);
        cb.tick();
    }

    return result;
}

std::unique_ptr<CircuitColorHandler>
VasculaturePopulationLoader::createColorHandler(brayns::ModelDescriptor *model,
                                                const std::string& config) const noexcept
{
    return std::make_unique<VasculatureColorHandler>(model, config, _population.name());
}
}
