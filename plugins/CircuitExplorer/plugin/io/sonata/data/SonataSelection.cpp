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

#include "SonataSelection.h"

#include <bbp/sonata/node_sets.h>
#include <bbp/sonata/report_reader.h>

#include <random>

NodeSelection::NodeSelection()
 : _nodeSetsSelection({})
 , _nodeListSelection({})
 , _simulationSelection({})
{
}

void NodeSelection::select(const bbp::sonata::CircuitConfig& config,
                           const std::string& population,
                           const std::vector<std::string>& nodeSets)
{
    const auto nodePopulation = config.getNodePopulation(population);

    if(!nodeSets.empty())
    {
        const auto nodeSetFile = bbp::sonata::NodeSets::fromFile(config.getNodeSetsPath());
        for(const auto& nodeSetName : nodeSets)
        {
            const auto newSelection = nodeSetFile.materialize(nodeSetName, nodePopulation);
            _nodeSetsSelection = _nodeListSelection & newSelection;
        }
    }
    else
        _nodeSetsSelection = nodePopulation.selectAll();
}

void NodeSelection::select(const std::vector<uint64_t>& nodeList)
{
    _nodeListSelection = bbp::sonata::Selection::fromValues(nodeList);
}

void NodeSelection::select(const SimulationType simType,
                           const std::string& reportPath,
                           const std::string& population)
{
    std::vector<bbp::sonata::NodeID> nodeIds;
    switch(simType)
    {
        case SimulationType::REPORT:
        {
            const bbp::sonata::ElementReportReader report (reportPath);
            nodeIds = report.openPopulation(population).getNodeIds();
            break;
        }
        default:
            return;
    }

    std::sort(nodeIds.begin(), nodeIds.end());
    _simulationSelection = bbp::sonata::Selection::fromValues(nodeIds);
}

bbp::sonata::Selection NodeSelection::intersection(const double percent)
{
    // Specified list of nodes have preference
    if(!_nodeListSelection.empty())
    {
        // If also report is specified, return only the intersection
        if(!_simulationSelection.empty())
            return _nodeListSelection & _simulationSelection;
        else
            return _nodeListSelection;
    }
    else
    {
        bbp::sonata::Selection common ({});
        if(!_simulationSelection.empty())
            common = _nodeSetsSelection & _simulationSelection;
        else
            common = _nodeSetsSelection;

        if(percent < 1.0)
        {
            auto commonList = common.flatten();

            std::random_device randomDevice;
            std::mt19937_64 randomEngine(randomDevice());
            std::shuffle(commonList.begin(), commonList.end(), randomEngine);
            const size_t finalSize = static_cast<size_t>(
                        percent * static_cast<double>(commonList.size()));
            commonList.resize(finalSize);
            std::sort(commonList.begin(), commonList.end());

            common = bbp::sonata::Selection::fromValues(commonList);
        }

        return common;
    }

}

EdgeSelection::EdgeSelection()
 : _selection({})
{
}

void EdgeSelection::select(const bbp::sonata::CircuitConfig& config,
                           const std::string& edgePopulation,
                           const bbp::sonata::Selection& nodeSelection,
                           const bool afferent)
{
    const auto& edges = config.getEdgePopulation(edgePopulation);
    _selection = afferent ? edges.afferentEdges(nodeSelection.flatten())
                          : edges.efferentEdges(nodeSelection.flatten());
}

const bbp::sonata::Selection& EdgeSelection::selection() const noexcept
{
    return _selection;
}
