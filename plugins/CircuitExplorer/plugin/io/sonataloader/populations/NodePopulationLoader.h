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

#include <bbp/sonata/config.h>

#include <plugin/api/CircuitColorHandler.h>
#include <plugin/io/morphology/MorphologyInstance.h>
#include <plugin/io/sonataloader/SonataLoaderProperties.h>
#include <plugin/io/util/ProgressReport.h>

namespace sonataloader
{
/**
 * @brief The NodePopulationLoader is the base class for implementations that load SONATA
 *        node populations which must be transformed into morphology instances (in other
 *        words, transforms node population data into scene geometry)
 */
class NodePopulationLoader
{
public:
    /**
     * @brief Initializes this loader to work with the given node population and its
     *        properties
     */
    NodePopulationLoader(bbp::sonata::NodePopulation&& population,
                         bbp::sonata::PopulationProperties&& properties)
     : _population(std::move(population))
     , _populationProperties(std::move(properties))
    {
    }

    virtual ~NodePopulationLoader() = default;

    /**
     * @brief load the node population data. The given parameters may be used to configure
     *        the load process. The SubProgressReport allows to notify progress to listening
     *        clients of the Brayns API
     */
    virtual std::vector<MorphologyInstancePtr>
    load(const PopulationLoadConfig& loadSettings,
         const bbp::sonata::Selection& nodeSelection,
         SubProgressReport& cb) const = 0;

    /**
     * @brief creates the appropiate CircuitColorHandler instance for the type of nodes this
     *        loader has read from disk
     */
    virtual std::unique_ptr<CircuitColorHandler>
    createColorHandler(brayns::ModelDescriptor*, const std::string& configPath) const noexcept = 0;

protected:
    bbp::sonata::NodePopulation _population;
    bbp::sonata::PopulationProperties _populationProperties;
};

using NodePopulationLoaderPtr = std::unique_ptr<NodePopulationLoader>;
}

