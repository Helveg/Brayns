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

#include "SonataSimulation.h"

#include <unordered_map>

class SonataSimulationFactory
{
public:
    static SonataSimulationFactory& instance() noexcept
    {
        static SonataSimulationFactory fact;
        return fact;
    }

    template<class T>
    void registerSimulation(const SimulationType type)
    {
        static_assert (std::is_base_of<SonataSimulation, T>::value,
                       "Tried to register a non SonataReport");

        _reports[type] = [](const std::string& path, const std::string& population)
        {
            return std::make_unique<T>(path, population);
        };
    }

    std::unique_ptr<SonataSimulation> createSimulation(const SimulationType type,
                                                       const std::string& path,
                                                       const std::string& population)
    {
        auto it = _reports.find(type);
        if(it == _reports.end())
            throw std::runtime_error("Non supported report type: '"
                                     + simulationTypeToString(type) + '"');

        return (it->second)(path, population);
    }
private:
    using ReportPtr = std::unique_ptr<SonataSimulation>;
    using LambdaFactory = std::function<ReportPtr(const std::string&, const std::string&)>;
    std::unordered_map<SimulationType, LambdaFactory> _reports;
};
