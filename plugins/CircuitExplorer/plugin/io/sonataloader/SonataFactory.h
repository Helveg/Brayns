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

#include <plugin/io/morphology/neuron/NeuronBuilder.h>
#include <plugin/io/morphology/neuron/NeuronGeometryType.h>
#include <plugin/io/sonataloader/populations/EdgePopulationLoader.h>
#include <plugin/io/sonataloader/populations/NodePopulationLoader.h>
#include <plugin/io/sonataloader/simulations/SimulationLoader.h>
#include <plugin/io/util/Factory.h>

namespace sonataloader
{
class SonataFactories
{
public:
    SonataFactories();

    const auto& neuronBuilders() const noexcept
    {
        return _neuronBuilders;
    }

    const auto& edgeLoaders() const noexcept
    {
        return _edgeLoaders;
    }

    const auto& nodeLoaders() const noexcept
    {
        return _nodeLoaders;
    }

    const auto& simulations() const noexcept
    {
        return _simulations;
    }

private:
    Factory<NeuronGeometryType, NeuronBuilder> _neuronBuilders;
    Factory<std::string,
            EdgePopulationLoader,
            const bbp::sonata::CircuitConfig&,
            const std::string&,
            const float&,
            const bool&> _edgeLoaders;
    Factory<std::string,
            NodePopulationLoader,
            bbp::sonata::NodePopulation,
            bbp::sonata::PopulationProperties> _nodeLoaders;
    Factory<SimulationType,
            SimulationLoader<NodeSimulationMapping>,
            const std::string&,
            const std::string&> _simulations;
};
}
