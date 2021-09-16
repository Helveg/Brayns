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

#include <brayns/common/loader/Loader.h>

class SubProgressReport
{
public:
    SubProgressReport(const brayns::LoaderProgress& cb,
                      const std::string& message,
                      const float start,
                      const float chunk,
                      const size_t numTicks);

    void tick() noexcept;

    void done() noexcept;
private:
    const brayns::LoaderProgress& _cb;
    const std::string _message;
    const float _start;
    const float _tick;
    const float _localTick;
    float _progress;
    float _localProgress;
};

class ProgressReport
{
public:
    ProgressReport(const brayns::LoaderProgress& cb,
                   const float start,
                   const float chunk,
                   const size_t numSubReports = 1);

    SubProgressReport nextSubProgress(const std::string& message,
                                      const size_t numTicks = 1) noexcept;
private:
    const brayns::LoaderProgress& _cb;
    const float _start;
    const float _subChunk;
    float _localChunk;
};
