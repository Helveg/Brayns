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

#include "SonataPopulation.h"

#include "../data/SonataSelection.h"

#include <bbp/sonata/node_sets.h>
#include <bbp/sonata/report_reader.h>

#include <algorithm>
#include <random>

namespace sonata
{
namespace population
{

namespace
{
bbp::sonata::Selection selectCells(const bbp::sonata::CircuitConfig& config,
                                   const PopulationLoadConfig& properties)
{
    data::SonataSelection selection;
    selection.select(config, properties.name, properties.nodeSets);
    selection.select(properties.nodeIds);
    //selection.select(reportType, reportPath, popName);
    return selection.intersection(properties.percentage);
}
}

SonataPopulation::SonataPopulation(const bbp::sonata::CircuitConfig& config,
                                   const PopulationLoadConfig& properties)
 : _config(config)
 , _nodePopulation(properties.name)
 , _selection(selectCells(config, properties))
{
    _loadCells();

    for(const auto& afferentP : properties.afferentPopulations)
        _loadAfferentSynapses(afferentP, properties.edgePercentage);
    for(const auto& efferentP : properties.efferentPopulations)
        _loadEfferentSynapses(efferentP, properties.edgePercentage);
}

const bbp::sonata::Selection& SonataPopulation::selectedNodes() const noexcept
{
    return _selection;
}

size_t SonataPopulation::getNumLoadedCells() const noexcept
{
    return _cells.size();
}

std::vector<uint64_t> SonataPopulation::getCellIds() const noexcept
{
    std::vector<uint64_t> result (_cells.size(), 0u);
    size_t i = 0;
    while(i < _cells.size())
        result[i] = _cells[i++].id;

    return result;
}

const data::Cell& SonataPopulation::getCell(const size_t index) const
{
    if(index >= _cells.size())
        throw std::out_of_range("Cell index " + std::to_string(index)
                                + " out of range of cell count: " + std::to_string(_cells.size()));

    return _cells[index];
}

const std::vector<data::Synapse>&
SonataPopulation::getAfferentSynapses(const data::Cell& cell,
                                      const std::string& edgePopulation) const
{
    const auto it = _afferentSynapses.find(edgePopulation);
    if(it == _afferentSynapses.end())
        throw std::invalid_argument("Afferent edge population " +
                                    edgePopulation + " has not been loaded");

    return it->second[cell.index];
}

const std::vector<data::Synapse>&
SonataPopulation::getEfferentSynapses(const data::Cell& cell,
                                      const std::string& edgePopulation) const
{
    const auto it = _efferentSynapses.find(edgePopulation);
    if(it == _efferentSynapses.end())
        throw std::invalid_argument("Efferent edge population " +
                                    edgePopulation + " has not been loaded");

    return it->second[cell.index];
}

void SonataPopulation::_loadCells()
{
    _cells = data::SonataCells::getCells(_config.getNodePopulation(_nodePopulation), _selection);
    if(_cells.empty())
        throw std::runtime_error("Node population " + _nodePopulation + ": no cells loaded!");
}

void SonataPopulation::_loadAfferentSynapses(const std::string& edgePopulation,
                                             const float percent)
{
    const auto edges = _config.getEdgePopulation(edgePopulation);

    // Make sure the node population is one of the 2 node networks that the requested
    // edge population is connecting
    if(edges.source() != _nodePopulation && edges.target() != _nodePopulation)
        throw std::runtime_error("The requested edge population '" + edgePopulation + "'"
                                 " is not linked to the node population '" + _nodePopulation
                                 + "'");

    _afferentSynapses[edgePopulation] = data::SonataSynapses::getAfferent(edges, _selection, percent);
}

void SonataPopulation::_loadEfferentSynapses(const std::string& edgePopulation,
                                             const float percent)
{
    const auto edges = _config.getEdgePopulation(edgePopulation);

    // Make sure the node population is one of the 2 node networks that the requested
    // edge population is connecting
    if(edges.source() != _nodePopulation && edges.target() != _nodePopulation)
        throw std::runtime_error("The requested edge population '" + edgePopulation + "'"
                                 " is not linked to the node population '" + _nodePopulation
                                 + "'");

    _efferentSynapses[edgePopulation] = data::SonataSynapses::getEfferent(edges, _selection, percent);
}
} // namespace data
} // namespace sonata
