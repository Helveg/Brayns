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

/**
 * @brief The SubProgressReport class implements functionality to track and report
 *        sub-progress of a large task
 */
class SubProgressReport
{
public:
    /**
     * @brief initializes this sub progress report
     */
    SubProgressReport(const brayns::LoaderProgress& cb,
                      const std::string& message,
                      const float start,
                      const float chunk,
                      const size_t numTicks);

    /**
     * @brief ticks a minimum progress unit, broadcasting information to the clients
     */
    void tick() noexcept;

    /**
     * @brief ticks a number of minimum progress unit as a batch, broadcasting information
     *        to the clients
     */
    void tickBatch(const size_t num) noexcept;
private:
    const brayns::LoaderProgress& _cb;
    const std::string _message;
    const float _start;
    const float _tick;
    float _progress;
};

/**
 * @brief The ProgressReport class implements functionality to track and report the
 *        progress of a loading task, allowing to split it in multiple chunks which can
 *        be tracked independently
 */
class ProgressReport
{
public:
    /**
     * @brief initializes the report progress of a task, splitting it by the number of subreports
     *        given
     */
    ProgressReport(const brayns::LoaderProgress& cb,
                   const float start,
                   const float chunk,
                   const size_t numSubReports = 1);

    /**
     * @brief creates the next subreport, based on the current state of the progress report
     */
    SubProgressReport nextSubProgress(const std::string& message,
                                      const size_t numTicks = 1) noexcept;
private:
    const brayns::LoaderProgress& _cb;
    const float _start;
    const float _subChunk;
    float _localChunk;
};
