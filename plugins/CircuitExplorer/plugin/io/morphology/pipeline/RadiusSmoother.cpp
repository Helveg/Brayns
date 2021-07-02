/* Copyright (c) 2015-2021, EPFL/Blue Brain Project
 * All rights reserved. Do not distribute without permission.
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

#include "RadiusSmoother.h"

#include <iostream>
#include <queue>

namespace morphology
{
namespace
{
#define MAX_RADIUS_PERCENT_CHANGE 0.07f

inline auto getSmoothRadius(float current, const float parentRadius)
{
    const auto maxChange = parentRadius * MAX_RADIUS_PERCENT_CHANGE;
    if(std::abs(current - parentRadius) > maxChange)
    {
        if(current > parentRadius)
            current = parentRadius + maxChange;
        else
            current = parentRadius - maxChange;
    }
    return current <= 0.f? parentRadius : current;
}
}

void RadiusSmoother::proccess(Morphology& morphology) const
{
    std::queue<Section*> smoothQueue;

    // If we have the soma, use half of its radius as starting smoothing radius
    if(morphology.hasSoma())
    {
        Soma& soma = morphology.soma();
        for(Section* s : soma.children)
        {
            s->samples.front().w = soma.radius * 0.4f;
            for(size_t i = 1; i < s->samples.size(); ++i)
            {
                auto& sample = s->samples[i];
                sample.w = getSmoothRadius(sample.w, s->samples[i - 1].w);
            }
            smoothQueue.push(s);
        }
    }
    // Otherwise, use root sections first sample radius as starting smoothing radius
    else
    {
        for(auto& section : morphology.sections())
        {
            // We look for root sections
            if(section.parentId != -1)
                continue;
            for(size_t i = 1; i < section.samples.size(); ++i)
            {
                auto& sample = section.samples[i];
                sample.w = getSmoothRadius(sample.w, section.samples[i - 1].w);
            }
            smoothQueue.push(&section);

        }
    }

    // Process the rest of the sections on a parent->child manner
    while(!smoothQueue.empty())
    {
        Section* s = smoothQueue.front(); smoothQueue.pop();

        auto children = morphology.sectionChildren(*s);
        for(auto child : children)
        {
            float prevRadius = s->samples.back().w;
            for(size_t i = 0; i < child->samples.size(); ++i)
            {
                auto& sample = child->samples[i];
                sample.w = getSmoothRadius(sample.w, prevRadius);
                prevRadius = sample.w;
            }

            smoothQueue.push(child);
        }
    }
}
}
