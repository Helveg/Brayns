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

#include <brayns/common/simulation/AbstractSimulationHandler.h>

#include <bbp/sonata/report_reader.h>

namespace sonata
{
class SonataReportHandler : public brayns::AbstractSimulationHandler
{
public:
    SonataReportHandler(const std::string& h5FilePath,
                        const std::string& populationName,
                        const bbp::sonata::Selection& selection);

    brayns::AbstractSimulationHandlerPtr clone() const final;

    void* getFrameDataImpl(const uint32_t frame) final;

    bool isReady() const final
    {
        return _ready;
    }

private:
    const std::string _h5FilePath;
    const std::string _populationName;
    const bbp::sonata::Selection _selection;
    std::unique_ptr<bbp::sonata::ElementReportReader::Population> _reportPopulation;
    bool _ready {false};
};
}
