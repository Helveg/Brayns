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

#include <plugin/io/sonata/populations/NodeColorHandler.h>

struct BiophysicalMaterialMap : public ElementMaterialMap
{
    size_t soma {std::numeric_limits<size_t>::max()};
    size_t axon {std::numeric_limits<size_t>::max()};
    size_t dendrite {std::numeric_limits<size_t>::max()};
    size_t apicalDendrite {std::numeric_limits<size_t>::max()};
};

/**
 * @brief The BiophysicalColorHandler class provides functionality
 *        to set a biophysical node population circuit color
 */
class BiophysicalColorHandler : public NodeColorHandler
{
public:
    BiophysicalColorHandler(brayns::ModelDescriptor* model,
                              const std::string& configPath,
                              const std::string& population);

    std::unordered_set<std::string> getAvailableMethods() const noexcept final;

    std::unordered_set<std::string> getMethodVariables(const std::string& method) const final;

    void updateColor(const std::string& method, const ColorVariables& variables) final;

private:
    std::unordered_set<std::string> _methodCache;
};
