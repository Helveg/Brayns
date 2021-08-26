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

#include "AggregateGroup.h"

void AggregateGroup::addGroup(const std::string& population, std::unique_ptr<SynapseGroup>&& group)
{
    _aggregation[population] = std::move(group);
}

void AggregateGroup::mapToCell(const MorphologyInstance& cell)
{
    for(const auto& groupEntry : _aggregation)
        groupEntry.second->mapToCell(cell);
}

void AggregateGroup::mapSimulation(const std::unordered_map<uint64_t, uint64_t>& mapping)
{
    for(const auto& groupEntry : _aggregation)
        groupEntry.second->mapSimulation(mapping);
}

void AggregateGroup::addToModel(brayns::Model& model) const
{
    for(const auto& groupEntry : _aggregation)
        groupEntry.second->addToModel(model);
}
