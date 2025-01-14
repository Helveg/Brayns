/* Copyright (c) 2015-2018, EPFL/Blue Brain Project
 * All rights reserved. Do not distribute without permission.
 * Responsible Author: Cyrille Favreau <cyrille.favreau@epfl.ch>
 *
 * This file is part of Brayns <https://github.com/BlueBrain/Brayns>
 *
 * Based on OSPRay implementation
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

#include "AdvancedSimulationRenderer.h"
#include <brayns/common/log.h>

// ospray
#include <ospray/SDK/common/Data.h>
#include <ospray/SDK/common/Model.h>

// ispc exports
#include "AdvancedSimulationRenderer_ispc.h"

using namespace ospray;

namespace brayns
{
void AdvancedSimulationRenderer::commit()
{
    SimulationRenderer::commit();

    _shadows = getParam1f("shadows", 0.f);
    _softShadows = getParam1f("softShadows", 0.f);
    _ambientOcclusionStrength = getParam1f("aoWeight", 0.f);
    _ambientOcclusionDistance = getParam1f("aoDistance", 1e20f);
    _shadingEnabled = getParam1i("shading", 0) == int(Shading::diffuse);
    _electronShadingEnabled =
        getParam1i("shading", 0) == int(Shading::electron);

    _randomNumber = getParam1i("randomNumber", 0);

    _samplingThreshold = getParam1f("samplingThreshold", 0.001f);
    _volumeSpecularExponent = getParam1f("volumeSpecularExponent", 20.f);
    _volumeAlphaCorrection = getParam1f("volumeAlphaCorrection", 0.5f);

    ispc::AdvancedSimulationRenderer_set(
        getIE(), (_bgMaterial ? _bgMaterial->getIE() : nullptr), _shadows,
        _softShadows, _ambientOcclusionStrength, _ambientOcclusionDistance,
        _shadingEnabled, _randomNumber, _timestamp, spp,
        _electronShadingEnabled, _lightPtr, _lightArray.size(),
        _simulationData ? (float*)_simulationData->data : NULL,
        _simulationDataSize, _samplingThreshold, _volumeSpecularExponent,
        _volumeAlphaCorrection);
}

AdvancedSimulationRenderer::AdvancedSimulationRenderer()
{
    ispcEquivalent = ispc::AdvancedSimulationRenderer_create(this);
}

OSP_REGISTER_RENDERER(AdvancedSimulationRenderer, advanced_simulation);
} // namespace brayns
