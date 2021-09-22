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

#include <brayns/engine/Model.h>

#include <bbp/sonata/edges.h>

#include <plugin/io/util/ProgressReport.h>
#include <plugin/io/sonataloader/SonataLoaderProperties.h>
#include <plugin/io/synapse/SynapseGroup.h>

namespace sonataloader
{
/**
 * @brief The EdgePopulationLoader is the base class for implementations that load SONATA
 *        edge populations which must be transformed into SynapseGroups (in other
 *        words, transforms edge population data into scene geometry)
 */
class EdgePopulationLoader
{
public:
    /**
     * @brief initializes this loader to work with the given population, in the given mode
     *        (afferent or not afferent) and with the given percentage of nodes to load
     */
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

    /**
     * @brief load the edge population data. The given parameters may be used to configure
     *        the load process. The SubProgressReport allows to notify progress to listening
     *        clients of the Brayns API
     */
    std::vector<std::unique_ptr<SynapseGroup>>
    virtual load(const PopulationLoadConfig& loadConfig,
                 const bbp::sonata::Selection& nodeSelection,
                 SubProgressReport& cb) const = 0;

    /**
     * @brief creates the appropiate CircuitColorHandler instance for the type of edges this
     *        loader has read from disk
     */
    virtual std::unique_ptr<CircuitColorHandler>
    createColorHandler(brayns::ModelDescriptor*, const std::string& configPath) const noexcept = 0;

protected:
    const bbp::sonata::CircuitConfig& _config;
    const bbp::sonata::EdgePopulation _population;
    const float _percentage;
    const bool _afferent;

};

using EdgePopulationLoaderPtr = std::unique_ptr<EdgePopulationLoader>;
}
