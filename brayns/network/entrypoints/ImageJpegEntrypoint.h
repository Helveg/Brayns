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

#include <brayns/network/adapters/ImageBase64Adapter.h>
#include <brayns/network/entrypoint/Entrypoint.h>

namespace brayns
{
class ImageJpegEntrypoint
    : public Entrypoint<EmptyMessage, ImageGenerator::ImageBase64>
{
public:
    virtual std::string getName() const override { return "image-jpeg"; }

    virtual std::string getDescription() const override
    {
        return "Take a snapshot at JPEG format";
    }

    virtual void onRequest(const Request& request) override
    {
        auto& api = getApi();
        auto& engine = api.getEngine();
        auto& framebuffer = engine.getFrameBuffer();
        auto& manager = api.getParametersManager();
        auto& parameters = manager.getApplicationParameters();
        auto compression = uint8_t(parameters.getJpegCompression());
        auto& generator = getImageGenerator();
        auto image = generator.createImage(framebuffer, "jpg", compression);
        request.reply(image);
    }
};
} // namespace brayns