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

#include "VasculatureReportLoader.h"

#include <plugin/io/sonataloader/simulations/handlers/SonataReportHandler.h>
#include <plugin/io/sonataloader/simulations/handlers/VasculatureRadiiHandler.h>

namespace sonataloader
{
std::vector<NodeSimulationMapping>
VasculatureReportLoader::loadMapping(const bbp::sonata::Selection& s) const
{
    const bbp::sonata::ElementReportReader reader (_path);
    const auto& reportPopulation = reader.openPopulation(_population);

    const auto timeData = reportPopulation.getTimes();
    const auto start = std::get<0>(timeData);
    const auto step = std::get<2>(timeData);
    auto frameData = reportPopulation.get(s, start, start + step);
    const auto& rawMapping = frameData.ids;

    // Sorted vasculature mapping indices
    std::map<uint64_t, size_t> sortedCompartmentsSize;
    for(size_t i = 0; i < rawMapping.size(); ++i)
        sortedCompartmentsSize[rawMapping[i].first] = i;

    // Flatten
    std::vector<NodeSimulationMapping> mapping (sortedCompartmentsSize.size());
    auto it = sortedCompartmentsSize.begin();
    size_t index = 0;
    while(it != sortedCompartmentsSize.end())
    {
        mapping[index].globalOffset = it->second;
        ++it;
        ++index;
    }

    return mapping;
}

brayns::AbstractSimulationHandlerPtr
VasculatureReportLoader::createSimulationHandler(const bbp::sonata::Selection& s) const
{
    return std::make_shared<SonataReportHandler>(_path, _population, s);
}

brayns::AbstractSimulationHandlerPtr
VasculatureRadiiReportLoader::createSimulationHandler(const bbp::sonata::Selection& s) const
{
    return std::make_shared<VasculatureRadiiHandler>(_path, _population, s);
}
}
