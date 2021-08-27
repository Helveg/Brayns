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

#include "SonataEndFeetReader.h"

#include <highfive/H5File.hpp>

#include <mutex>

#include <common/log.h>

std::vector<brayns::TriangleMesh>
SonataEndFeetReader::readEndFeet(const std::string& filePath,
                                 const std::vector<uint64_t>& ids,
                                 const std::vector<brayns::Vector3f>& positions)
{
    static std::mutex hdf5Mutex;
    std::unique_ptr<HighFive::File> file;
    {
        std::lock_guard<std::mutex> lock(hdf5Mutex);
        file = std::make_unique<HighFive::File>(filePath);
    }

    const auto root = file->getGroup("/objects");

    std::vector<brayns::TriangleMesh> result (ids.size());

    PLUGIN_WARN << "SURFACE POSITIONS ARE NOT BEING APPLIED TO ENDFEET MESHES" << std::endl;

    for(size_t i = 0; i < ids.size(); ++i)
    {
        auto& mesh = result[i];

        const auto endFootGroupName = "endfoot_" + std::to_string(ids[i]);
        const auto endFootGroup = root.getGroup(endFootGroupName);

        const auto vertexDataSet = endFootGroup.getDataSet("points");
        std::vector<std::vector<float>> rawVertices;
        vertexDataSet.select({0, 0}, vertexDataSet.getDimensions()).read(rawVertices);

        mesh.vertices.resize(rawVertices.size());
        for(size_t j = 0; j < rawVertices.size(); ++j)
        {
            //const auto& pos = positions[i];
            mesh.vertices[j].x = rawVertices[j][0];// + pos.x;
            mesh.vertices[j].y = rawVertices[j][1];// + pos.y;
            mesh.vertices[j].z = rawVertices[j][2];// + pos.z;
        }

        const auto triangleDataSet = endFootGroup.getDataSet("triangles");
        std::vector<std::vector<uint32_t>> rawTriangles;
        triangleDataSet.select({0, 0}, triangleDataSet.getDimensions()).read(rawTriangles);

        mesh.indices.resize(rawTriangles.size());
        for(size_t j = 0; j < rawTriangles.size(); ++j)
        {
            mesh.indices[j].x = rawTriangles[j][0];
            mesh.indices[j].y = rawTriangles[j][1];
            mesh.indices[j].z = rawTriangles[j][2];
        }
    }

    return result;
}

