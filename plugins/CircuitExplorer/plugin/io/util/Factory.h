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

// No easy way before C++17 to compile-time check wether Key is hashable or not

/**
 * @brief The Factory class implements templated factory objects which can be used
 *        to register and instantiate products from a given type of key
 */
template<class Key, class Product, typename ...ProductArgs>
class Factory
{
public:
    /**
     * @brief registers an instantiable product into the factory for the given key
     */
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

    /**
     * @brief attempts to instantiate a product which is associated with the given key
     * @throws std::invalid_argument if there is not product associated with the given key
     */
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
