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

#include "EdgeCompartmentLoader.h"

#include <plugin/io/sonata/simulations/handlers/SonataReportHandler.h>

#include <bbp/sonata/report_reader.h>

EdgeCompartmentLoader::EdgeCompartmentLoader(const std::string& path,
                                             const std::string& population)
 : SimulationLoader<EdgeSimulationMapping>(path, population)
{
    const bbp::sonata::ElementReportReader reader (path);
    const auto pops = reader.getPopulationNames();

    auto it = std::find_if(pops.begin(), pops.end(), [&](const std::string& p)
    {
        return p == population;
    });

    if(it == pops.end())
        throw std::runtime_error("Synapse simulation " + path + " does not have a population "
                                 "'" + population + "'");
}

std::vector<EdgeSimulationMapping>
EdgeCompartmentLoader::loadMapping(const bbp::sonata::Selection& s) const
{
    const bbp::sonata::ElementReportReader reader (_path);
    const auto& reportPopulation = reader.openPopulation(_population);

    const auto timeData = reportPopulation.getTimes();
    const auto start = std::get<0>(timeData);
    const auto step = std::get<2>(timeData);
    auto frameData = reportPopulation.get(s, start, start + step);
    const auto& rawMapping = frameData.ids;

    // Pre-fill the map so all node Ids will have their mapping,
    // even if these nodes are not reported on the simulation
    std::map<uint64_t, EdgeSimulationMapping> sortedCompartmentsSize;
    for(const auto nodeId : s.flatten())
        sortedCompartmentsSize[nodeId] = {};

    // Gather mapping
    uint64_t offset = 0;
    for(const auto& key : rawMapping)
    {
        auto& nodeMapping = sortedCompartmentsSize[key.first];
        nodeMapping.offsets[key.second] = offset++;
    }

    // Flatten
    std::vector<EdgeSimulationMapping> mapping (sortedCompartmentsSize.size());
    auto it = sortedCompartmentsSize.begin();
    for(size_t index = 0; it != sortedCompartmentsSize.end() ; ++it, ++index)
        mapping[index] = std::move(it->second);

    return mapping;
}

brayns::AbstractSimulationHandlerPtr
EdgeCompartmentLoader::createSimulationHandler(const bbp::sonata::Selection& s) const
{
    return std::make_shared<SonataReportHandler>(_path,
                                                 _population,
                                                 s);
}
