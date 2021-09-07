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

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>

#include "morphology/neuron/NeuronBuilder.h"
#include "populations/EdgePopulationLoader.h"
#include "populations/NodePopulationLoader.h"
#include "simulations/SimulationLoader.h"

// No easy way before C++17 to compile-time check wether Key is hashable or not
template<class Key, class Product, typename ...ProductArgs>
class SonataFactory
{
public:
    template<class DerivedProduct>
    void registerProduct(const Key& key)
    {
        if(_factories.find(key) != _factories.end())
            throw std::invalid_argument("Duplicate key in factory");

        _factories[key] = [](ProductArgs&& ...args)
        {
            return std::make_unique<DerivedProduct>(std::forward<ProductArgs>(args)...);
        };
    }

    std::unique_ptr<Product> instantiate(const Key& key, ProductArgs&& ...args) const
    {
        auto it = _factories.find(key);
        if(it == _factories.end())
            throw std::invalid_argument("Key not found in factory list");

        return it->second(std::forward<ProductArgs>(args)...);
    }

private:
    using ProductFactory = std::function<std::unique_ptr<Product>(ProductArgs&& ...args)>;
    std::unordered_map<Key, ProductFactory> _factories;
};

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
    SonataFactory<std::string, NeuronBuilder> _neuronBuilders;
    SonataFactory<std::string,
                  EdgePopulationLoader,
                  const bbp::sonata::CircuitConfig&,
                  const std::string&,
                  const float&,
                  const bool&> _edgeLoaders;
    SonataFactory<std::string,
                  NodePopulationLoader,
                  bbp::sonata::NodePopulation,
                  bbp::sonata::PopulationProperties> _nodeLoaders;
    SonataFactory<SimulationType,
                  SimulationLoader<NodeSimulationMapping>,
                  const std::string&,
                  const std::string&> _simulations;
};
