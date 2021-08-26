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

#include "EndFootGroup.h"

void EndFootGroup::addSynapse(const uint64_t id,
                              const brayns::Vector3f& position,
                              brayns::TriangleMesh&& endFootMesh)
{
    _meshes.emplace_back(std::move(endFootMesh));
    auto& mesh = _meshes.back();
    for(auto& vertex : mesh.vertices)
        vertex += position;
}

void EndFootGroup::mapToCell(const MorphologyInstance&)
{
    // nothing to do for endfeet
}

void EndFootGroup::mapSimulation(const std::unordered_map<uint64_t, uint64_t>&)
{
    // mesh simulation not supported yet
}

void EndFootGroup::addToModel(brayns::Model& model) const
{
    for(auto& mesh : _meshes)
    {
        const auto matId = model.getMaterials().size();
        model.createMaterial(matId, "");
        model.getTriangleMeshes()[matId] = std::move(mesh);
    }
}
