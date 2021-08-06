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

#pragma once

#include <brayns/common/loader/Loader.h>
#include <brayns/engine/Model.h>

#include <bbp/sonata/edges.h>

#include "../SonataLoaderProperties.h"
#include "../simulations/SonataSimulation.h"

struct SynapseInfo
{
    uint64_t synapseId;
    int32_t sectionId;
    float distance;
    brayns::Vector3f position;
};

class EdgePopulationLoader
{
public:
    EdgePopulationLoader(bbp::sonata::EdgePopulation&& population,
                         bbp::sonata::PopulationProperties&& populationProperties)
     : _population(std::move(population))
     , _populationProperties(std::move(populationProperties))
    {
    }

    virtual ~EdgePopulationLoader() = default;

    virtual std::vector<std::vector<SynapseInfo>>
    load(const PopulationLoadConfig& loadConfig,
         const bbp::sonata::Selection& nodeSelection,
         const bool afferent) const = 0;

protected:
    bbp::sonata::EdgePopulation _population;
    bbp::sonata::PopulationProperties _populationProperties;

};

using EdgePopulationLoaderPtr = std::unique_ptr<EdgePopulationLoader>;
