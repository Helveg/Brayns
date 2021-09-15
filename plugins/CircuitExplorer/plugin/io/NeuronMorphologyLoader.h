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
 * @brief Imports a circuit from a BBP's BlueConfig/CircuitConfig file
 */
class NeuronMorphologyLoader : public brayns::Loader
{
public:
    /**
     * @brief Constructor
     */
    NeuronMorphologyLoader(brayns::Scene& scene);

    /**
     * @brief getSupportedExtensions returns a list with supported file extensions
     */
    std::vector<std::string> getSupportedExtensions() const final;

    /**
     * @brief isSupported returns wether the given filename or file extension are supported
     *        by this loader
     */
    bool isSupported(const std::string& filename, const std::string& extension) const final;

    /**
     * @brief getName returns the name of the loader, used when invoking this loader over
     *        the brayns API
     */
    std::string getName() const final;

    /**
     * @brief getProperties return all the available properties for this loader, and the default
     *        value of each of them
     */
    brayns::PropertyMap getProperties() const;

    /**
     * @brief importFromBlob imports models from a byte blob. NOT SUPPORTED
     */
    std::vector<brayns::ModelDescriptorPtr>
    importFromBlob(brayns::Blob&&,
                   const brayns::LoaderProgress&,
                   const brayns::PropertyMap&) const final;

    /**
     * @brief importFromFile imports models from a file given by a path
     */
    std::vector<brayns::ModelDescriptorPtr>
    importFromFile(const std::string& path,
                   const brayns::LoaderProgress& callback,
                   const brayns::PropertyMap& properties) const final;
};
