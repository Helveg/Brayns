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

#include "SonataSimulationMapping.h"

#include "../simulationhandlers/SonataReportHandler.h"

namespace sonata
{
namespace data
{
std::vector<SimulationMapping> ReportMapping::compute() const
{
    const bbp::sonata::ElementReportReader reader (_filePath);
    const auto reportPopulation = reader.openPopulation(_population);

    const auto timeData = reportPopulation.getTimes();
    const auto start = std::get<0>(timeData);
    const auto step = std::get<2>(timeData);
    auto frameData = reportPopulation.get(_selection, start, start + step);
    const auto& mapping = frameData.ids;

    // Compact mapping from libsonata
    std::map<uint64_t, std::vector<uint16_t>> sortedCompartmentsSize;
    int32_t lastSection = -1;
    uint64_t lastNode = std::numeric_limits<uint64_t>::max();
    for(const auto& key : mapping)
    {
        auto& cm = sortedCompartmentsSize[key.first];
        if(lastSection != key.second || lastNode != key.first)
        {
            lastNode = key.first;
            lastSection = key.second;
            cm.push_back(0u);
        }
        cm[key.second]++;
    }

    // Returns a node id sorted list of compartment mappings
    std::vector<SimulationMapping> result(sortedCompartmentsSize.size());
    // Transform into brayns mapping
    auto it = sortedCompartmentsSize.begin();
    size_t index = 0;
    size_t prevOffset = 0;
    for(; it != sortedCompartmentsSize.end(); ++it)
    {
        auto& cellMapping = result[index];
        cellMapping.globalOffset = prevOffset;
        cellMapping.sectionsCompartments.resize(it->second.size());
        cellMapping.sectionsOffsets.resize(it->second.size());

        uint16_t localOffset = 0;
        for(size_t i = 0; i < it->second.size(); ++i)
        {
            const auto sectionCompartments = it->second[i];
            cellMapping.sectionsOffsets[i] = localOffset;
            cellMapping.sectionsCompartments[i] = sectionCompartments;
            localOffset += sectionCompartments;
            prevOffset += sectionCompartments;
        }

        ++index;
    }

    return result;
}

brayns::AbstractSimulationHandlerPtr ReportMapping::createSimulationHandler() const
{
    return std::make_shared<SonataReportHandler>(_filePath, _population, _selection);
}

std::vector<SimulationMapping> SpikeMapping::compute() const
{
    std::vector<SimulationMapping> result (_selection.flatSize());
    for(size_t i = 0; i < result.size(); ++i)
        result[i].globalOffset = i;
    return result;
}

brayns::AbstractSimulationHandlerPtr SpikeMapping::createSimulationHandler() const
{
    return {nullptr};
}
}
}
