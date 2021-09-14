/* Copyright (c) 2015-2021, EPFL/Blue Brain Project
 * All rights reserved. Do not distribute without permission.
 * Authors: Cyrille Favreau <cyrille.favreau@epfl.ch>
 *          Nadir Roman Guerrero <nadir.romanguerrero@epfl.ch>
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

#include <brayns/common/types.h>
#include <brayns/pluginapi/ExtensionPlugin.h>

#include <plugin/api/CircuitColorManager.h>
#include <plugin/api/CircuitExplorerParams.h>

/**
 * @brief The CircuitExplorerPlugin class manages the loading and visualization
 * of the neuronscientific data (neuronal circuits, synapses, simulations, etc.)
 */
class CircuitExplorerPlugin : public brayns::ExtensionPlugin
{
public:
    CircuitExplorerPlugin();

    void init() final;

    /**
     * @brief preRender Updates the scene according to latest data load
     */
    void preRender() final;
    void postRender() final;

private:
    // Custom camera handling (TODO: Move to core)
    brayns::Message _setCamera(const CameraDefinition&);
    CameraDefinition _getCamera();

    // Material handling (TODO: Move to core)
    MaterialIds _getMaterialIds(const ModelId& modelId);
    MaterialDescriptor _getMaterial(const ModelMaterialId& mmId);
    brayns::Message _setMaterial(const MaterialDescriptor&);
    brayns::Message _setMaterials(const MaterialsDescriptor&);
    brayns::Message _setMaterialRange(const MaterialRangeDescriptor&);
    brayns::Message _setMaterialExtraAttributes(const MaterialExtraAttributes&);
    MaterialProperties _getMaterialProperties();
    brayns::Message _updateMaterialProperties(const UpdateMaterialProperties&);

    // Movie production (TODO: MOve to core)
    brayns::Message _exportFramesToDisk(const ExportFramesToDisk& payload);
    void _doExportFrameToDisk();
    FrameExportProgress _getFrameExportProgress();
    ExportLayerToDiskResult _exportLayerToDisk(const ExportLayerToDisk& payload);
    brayns::Message _makeMovie(const MakeMovieParameters& params);

    // Anterograde tracing
    brayns::Message _traceAnterogrades(const AnterogradeTracing& payload);

    // Geometry
    AddShapeResult _addSphere(const AddSphere& payload);
    AddShapeResult _addPill(const AddPill& payload);
    AddShapeResult _addCylinder(const AddCylinder& payload);
    AddShapeResult _addBox(const AddBox& payload);
    brayns::Message _addGrid(const AddGrid& payload);
    brayns::Message _addColumn(const AddColumn& payload);

    // Model geoemtry manipulation
    brayns::Message _mirrorModel(const MirrorModel& payload);
    brayns::Message _changeCircuitThickness(const CircuitThickness& payload);

    brayns::Message _colorCircuitById(const RequestColorCircuitById& payload);
    brayns::Message _colorCircuitBySingleColor(const RequestColorCircuitBySingleColor& payload);
    CircuitColorMethods _requestCircuitColorMethods(const ModelId& payload);
    CircuitColorMethodVariables
    _requestCircuitColorMethodVariables(const RequestCircuitColorMethodVariables& payload);
    brayns::Message _colorCircuitByMethod(const RequestColorCircuitByMethod& payload);

private:
    bool _dirty{false};

    ExportFramesToDisk _exportFramesToDiskPayload;
    bool _exportFramesToDiskDirty{false};
    // Flag used to avoid the first frame to be rendered with the wrong camera parameters
    bool _exportFramesToDiskStartFlag{false};
    uint16_t _frameNumber{0};
    uint32_t _accumulationFrameNumber{0};
    size_t _prevAccumulationSetting;
    bool _exportFrameError {false};
    std::string _exportFrameErrorMessage;

    CircuitColorManager _colorManager;
};
