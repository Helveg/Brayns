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

#include "ProgressReport.h"

#include <plugin/api/Log.h>

SubProgressReport::SubProgressReport(const brayns::LoaderProgress& cb,
                                     const std::string& message,
                                     const float start,
                                     const float chunk,
                                     const size_t numTicks)
 : _cb(cb)
 , _message(message)
 , _start(start)
 , _tick(chunk / static_cast<float>(numTicks))
 , _localTick(1.f / static_cast<float>(numTicks))
 , _progress(0.f)
 , _localProgress(0.f)
{
    _cb.updateProgress(_message, _start);
}

void SubProgressReport::tick() noexcept
{
    _progress += _tick;
    _cb.updateProgress(_message, _start + _progress);
    _localProgress += _localTick;
    PLUGIN_PROGRESS(static_cast<uint32_t>(_localProgress * 100.0), _message);
}

void SubProgressReport::done() noexcept
{
    PLUGIN_PROGRESS(100u, _message);
    PLUGIN_PROGRESS_DONE;
}

ProgressReport::ProgressReport(const brayns::LoaderProgress& cb,
                               const float start,
                               const float chunk,
                               const size_t numSubReports)
 : _cb(cb)
 , _start(start)
 , _subChunk(chunk / static_cast<float>(numSubReports))
 , _localChunk(0.f)
{
}

SubProgressReport ProgressReport::nextSubProgress(const std::string& msg,
                                                  const size_t numTicks) noexcept
{
    const float start = _start + _localChunk;
    _localChunk += _subChunk;
    return SubProgressReport(_cb, msg, start, _subChunk, numTicks);
}
