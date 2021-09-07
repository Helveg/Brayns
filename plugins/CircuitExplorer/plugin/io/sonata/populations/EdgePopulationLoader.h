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

#pragma once

#include <brayns/common/loader/Loader.h>
#include <brayns/engine/Model.h>

#include <bbp/sonata/edges.h>

#include <plugin/io/sonata/SonataLoaderProperties.h>
#include <plugin/io/sonata/synapse/SynapseGroup.h>

class EdgePopulationLoader
{
public:
    EdgePopulationLoader(const bbp::sonata::CircuitConfig& config,
                         const std::string& population,
                         const float percentage,
                         const bool afferent)
     : _config(config)
     , _population(_config.getEdgePopulation(population))
     , _percentage(percentage)
     , _afferent(afferent)
    {
    }

    virtual ~EdgePopulationLoader() = default;

    std::vector<std::unique_ptr<SynapseGroup>>
    virtual load(const PopulationLoadConfig& loadConfig,
                 const bbp::sonata::Selection& nodeSelection) const = 0;

    virtual std::unique_ptr<CircuitColorHandler>
    createColorHandler(brayns::ModelDescriptor*, const std::string& configPath) const noexcept = 0;

protected:
    bbp::sonata::Selection
    _applyPercentage(const bbp::sonata::Selection& srcEdgeSelection) const noexcept
    {
        if(_percentage >= 1.f)
            return srcEdgeSelection;

        const auto edgeIds = srcEdgeSelection.flatten();

        const auto chunk = static_cast<size_t>(edgeIds.size() * _percentage);
        std::vector<uint64_t> result;
        result.reserve(chunk);
        for(size_t i = 0; i < edgeIds.size(); i = i + chunk)
            result.push_back(edgeIds[i]);

        return bbp::sonata::Selection::fromValues(result);
    }

protected:
    const bbp::sonata::CircuitConfig& _config;
    const bbp::sonata::EdgePopulation _population;
    const float _percentage;
    const bool _afferent;

};

using EdgePopulationLoaderPtr = std::unique_ptr<EdgePopulationLoader>;
