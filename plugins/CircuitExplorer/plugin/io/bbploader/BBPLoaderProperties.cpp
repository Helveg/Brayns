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

#include "BBPLoaderProperties.h"

#include <brion/target.h>

#include <brayns/common/utils/filesystem.h>

namespace bbploader
{
namespace
{
using string_list = std::vector<std::string>;

void checkPropertiesExist(const brayns::PropertyMap& input, const brayns::PropertyMap& ref)
{
    for(const auto& property : ref.getProperties())
    {
        if(!input.hasProperty(property->name))
            throw std::invalid_argument("BBPLoader: Missing loader property '"
                                        + property->name + "'");
    }
}

void checkTargets(const brion::BlueConfig& config, const string_list& targetList)
{
    if(targetList.empty())
        return;

    const auto targetParsers = config.getTargets();
    for(const auto& trg : targetList)
    {
        bool exists = false;
        for(const auto& parser : targetParsers)
        {
            if(parser.contains(trg))
            {
                exists = true;
                break;
            }
        }
        if(!exists)
            throw std::invalid_argument("BBPLoader: Invalid or empty target: '" + trg + "'");
    }
}

void checkReport(const brion::BlueConfig& config,
                 const std::string& reportName,
                 const SimulationType reportType)
{
    switch(reportType)
    {
        case SimulationType::SPIKES:
        {
            const auto uri = config.getSpikeSource();
            if(uri.getPath().empty() || !fs::exists(uri.getPath()))
                throw std::invalid_argument("BBPLoader: Unable to find Spike report file");
            break;
        }
        case SimulationType::COMPARTMENT:
        {
            const auto uri = config.getReportSource(reportName);
            if(uri.getPath().empty() || !fs::exists(uri.getPath()))
                throw std::invalid_argument("BBPLoader: Unable to find Voltage report file for '"
                                            + reportName + "'");
            break;
        }
        case SimulationType::NONE:
            break;
    }
}

void checkRadiiMods(const float radMult, const float radOverriden)
{
    if(radMult <= 0.f)
        throw std::invalid_argument("BBPLoader: Invalid radius multiplier (Must be > 0.0)");

    if(radOverriden < 0.f)
        throw std::invalid_argument("BBPLoader: Invalid radius override (Must be > 0.0)");
}

} // namespace

brayns::PropertyMap BBPLoaderProperties::getPropertyList() noexcept
{
    brayns::PropertyMap result;
    result.setProperty(PROP_PERCENTAGE);
    result.setProperty(PROP_TARGETS);
    result.setProperty(PROP_GIDS);
    result.setProperty(PROP_REPORT);
    result.setProperty(PROP_REPORT_TYPE);
    result.setProperty(PROP_SPIKE_TRANSITION_TIME);
    result.setProperty(PROP_GEOMETRY_MODE);
    result.setProperty(PROP_RADIUS_MULTIPLIER);
    result.setProperty(PROP_RADIUS_OVERRIDE);
    result.setProperty(PROP_LOAD_SOMA);
    result.setProperty(PROP_LOAD_AXON);
    result.setProperty(PROP_LOAD_DENDRITE);
    result.setProperty(PROP_LOAD_APICAL_DENDRITE);
    result.setProperty(PROP_LOAD_AFFERENT_SYNAPSES);
    result.setProperty(PROP_LOAD_EFFERENT_SYNAPSES);
    return result;
}

BBPCircuitLoadConfig BBPLoaderProperties::checkAndParse(const brion::BlueConfig& config,
                                                        const brayns::PropertyMap& input)
{
    checkPropertiesExist(input, getPropertyList());

    BBPCircuitLoadConfig result;

    // Cells to load
    result.percentage = static_cast<float>(input.getProperty<double>(PROP_PERCENTAGE.name));
    if(result.percentage < 0.f)
        throw std::invalid_argument("BBPLoader: A negative percentage of cells is not allowed");
    result.percentage = std::min(result.percentage, 1.f);

    result.targets = brayns::string_utils::split(
                input.getPropertyRef<std::string>(PROP_TARGETS.name), ',');
    checkTargets(config, result.targets);
    const auto gidStrs = brayns::string_utils::split(
                input.getPropertyRef<std::string>(PROP_GIDS.name), ',');
    if(!gidStrs.empty())
    {
        result.gids.reserve(gidStrs.size());
        for(const auto& gidStr : gidStrs)
        {
            try
            {
                result.gids.push_back(std::stoull(gidStr));
            }
            catch(const std::exception& e)
            {
                throw std::invalid_argument("Could not parse GID '" + gidStr +
                                            "': " + std::string(e.what()));
            }
        }
    }

    // Simulation parameters
    result.reportName = input.getPropertyRef<std::string>(PROP_REPORT.name);
    result.reportType = EnumWrapper<SimulationType>().fromString(
                input.getPropertyRef<std::string>(PROP_REPORT_TYPE.name));
    checkReport(config, result.reportName, result.reportType);
    result.spikeTransitionTime = static_cast<float>(
                input.getProperty<double>(PROP_SPIKE_TRANSITION_TIME.name));
    if(result.spikeTransitionTime < 0.f)
        throw std::invalid_argument("BBPLoader: 'spike_transition_time' must be a positive");

    // Neuron morphology parameters
    result.radiusMultiplier = static_cast<float>(
                input.getProperty<double>(PROP_RADIUS_MULTIPLIER.name));
    result.radiusOverride = static_cast<float>(
                input.getProperty<double>(PROP_RADIUS_OVERRIDE.name));
    checkRadiiMods(result.radiusMultiplier, result.radiusOverride);

    result.morphologySections = NeuronSection::NONE;
    const auto loadSoma = input.getProperty<bool>(PROP_LOAD_SOMA.name);
    const auto loadAxon = input.getProperty<bool>(PROP_LOAD_AXON.name);
    const auto loadDend = input.getProperty<bool>(PROP_LOAD_DENDRITE.name);
    const auto loadADen = input.getProperty<bool>(PROP_LOAD_APICAL_DENDRITE.name);
    if(loadSoma)
        result.morphologySections |= NeuronSection::SOMA;
    if(loadAxon)
        result.morphologySections |= NeuronSection::AXON;
    if(loadDend)
        result.morphologySections |= NeuronSection::DENDRITE;
    if(loadADen)
        result.morphologySections |= NeuronSection::APICAL_DENDRITE;

    if(result.morphologySections == NeuronSection::SOMA)
        result.geometryMode = NeuronGeometryType::SAMPLES;
    else
        result.geometryMode = EnumWrapper<NeuronGeometryType>().fromString(
                input.getPropertyRef<std::string>(PROP_GEOMETRY_MODE.name));

    // Synapse parameters
    result.loadAfferent = input.getProperty<bool>(PROP_LOAD_AFFERENT_SYNAPSES.name);
    result.loadEfferent = input.getProperty<bool>(PROP_LOAD_EFFERENT_SYNAPSES.name);

    return result;
}
}
