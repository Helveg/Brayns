/* Copyright (c) 2015-2021 EPFL/Blue Brain Project
 * All rights reserved. Do not distribute without permission.
 *
 * Responsible Author: adrien.fleury@epfl.ch
 *
 * This file is part of Brayns <https://github.com/BlueBrain/Brayns>
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

#include <brayns/network/json/Message.h>

namespace dti
{
BRAYNS_MESSAGE_BEGIN(SetSpikeSimulationFromFileMessage)
BRAYNS_MESSAGE_ENTRY(size_t, model_id, "The ID of the loaded model")
BRAYNS_MESSAGE_ENTRY(double, dt, "Simulation time step")
BRAYNS_MESSAGE_ENTRY(double, time_scale, "Simulation time scale")
BRAYNS_MESSAGE_ENTRY(double, decay_speed, "Speed of spike decay")
BRAYNS_MESSAGE_ENTRY(double, rest_intensity, "Rest intensity")
BRAYNS_MESSAGE_ENTRY(double, spike_intensity, "Spike intensity")
BRAYNS_MESSAGE_ENTRY(std::string, path, "Path to BlueConfig file")
BRAYNS_MESSAGE_END()
} // namespace dti