# Copyright (c) 2015-2019, EPFL/Blue Brain Project
# All rights reserved. Do not distribute without permission.
#
# This file is part of Brayns <https://github.com/BlueBrain/Brayns>

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(braynsCircuitExplorer VERSION 0.1.0)
set(braynsCircuitExplorer_VERSION_ABI 1)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake
                              ${CMAKE_SOURCE_DIR}/CMake/common)
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/CMake/common/Common.cmake)
  message(FATAL_ERROR "CMake/common missing, run: git submodule update --init --recursive")
endif()

include(Common)
common_find_package(sonata)
common_find_package(Brion)
common_find_package(ospray 1.8 SYSTEM)
common_find_package(CGAL)
common_find_package(OpenMP)
common_find_package_post()

# reuse ispc setup and macros from ospray
list(APPEND CMAKE_MODULE_PATH ${OSPRAY_CMAKE_ROOT})
if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set(OSPRAY_DEBUG_BUILD ON)
endif()
include(ispc)

set(BRAYNSCIRCUITEXPLORER_SOURCES
    module/CircuitExplorer.cpp
    module/ispc/camera/DOFPerspectiveCamera.cpp
    module/ispc/camera/SphereClippingPerspectiveCamera.cpp
    module/ispc/render/CircuitExplorerMaterial.cpp
    module/ispc/render/CircuitExplorerBasicRenderer.cpp
    module/ispc/render/CircuitExplorerAdvancedRenderer.cpp
    module/ispc/render/VoxelizedSimulationRenderer.cpp
    module/ispc/render/ProximityDetectionRenderer.cpp
    module/ispc/render/CellGrowthRenderer.cpp
    module/ispc/render/utils/CircuitExplorerAbstractRenderer.cpp
    module/ispc/render/utils/CircuitExplorerSimulationRenderer.cpp
    plugin/api/CellObjectMapper.cpp
    plugin/meshing/MetaballsGenerator.cpp
    plugin/meshing/PointCloudMesher.cpp
    plugin/CircuitExplorerPlugin.cpp
    plugin/io/VoltageSimulationHandler.cpp
    plugin/io/CellGrowthHandler.cpp
    plugin/io/SpikeSimulationHandler.cpp
    plugin/io/MorphologyCollageLoader.cpp
    plugin/io/PairSynapsesLoader.cpp
    plugin/io/AbstractCircuitLoader.cpp
    plugin/io/MeshCircuitLoader.cpp
    plugin/io/AdvancedCircuitLoader.cpp
    plugin/io/AstrocyteLoader.cpp
    plugin/io/SynapseCircuitLoader.cpp
    plugin/io/BrickLoader.cpp
    plugin/io/MorphologyLoader.cpp
    plugin/io/SynapseJSONLoader.cpp
    plugin/io/Utils.cpp
    plugin/io/sonata/SonataFactory.cpp
    plugin/io/sonata/SonataLoader.cpp
    plugin/io/sonata/SonataLoaderProperties.cpp
    plugin/io/sonata/SonataNGVLoader.cpp
    plugin/io/sonata/data/SonataCells.cpp
    plugin/io/sonata/data/SonataSelection.cpp
    plugin/io/sonata/data/SonataSynapses.cpp
    plugin/io/sonata/data/SonataVasculature.cpp
    plugin/io/sonata/morphology/neuron/NeuronMorphology.cpp
    plugin/io/sonata/morphology/neuron/builders/PrimitiveNeuronBuilder.cpp
    plugin/io/sonata/morphology/neuron/builders/SDFNeuronBuilder.cpp
    plugin/io/sonata/morphology/neuron/builders/SampleNeuronBuilder.cpp
    plugin/io/sonata/morphology/neuron/instances/PrimitiveNeuronInstance.cpp
    plugin/io/sonata/morphology/neuron/instances/SDFNeuronInstance.cpp
    plugin/io/sonata/morphology/neuron/instances/SampleNeuronInstance.cpp
    plugin/io/sonata/morphology/neuron/pipeline/RadiusMultiplier.cpp
    plugin/io/sonata/morphology/neuron/pipeline/RadiusSmoother.cpp
    plugin/io/sonata/morphology/neuron/pipeline/SectionWelder.cpp
    plugin/io/sonata/morphology/vasculature/VasculatureSDFBuilder.cpp
    plugin/io/sonata/morphology/vasculature/VasculatureSDFInstance.cpp
    plugin/io/sonata/populations/edges/CommonEdgePopulationLoader.cpp
    plugin/io/sonata/populations/edges/EndFootPopulationLoader.cpp
    plugin/io/sonata/populations/edges/SynapseAstrocytePopulationLoader.cpp
    plugin/io/sonata/populations/nodes/AstrocytePopulationLoader.cpp
    plugin/io/sonata/populations/nodes/BiophysicalPopulationLoader.cpp
    plugin/io/sonata/populations/nodes/VasculaturePopulationLoader.cpp
    plugin/io/sonata/simulations/TransferFunctionSetUp.cpp
    plugin/io/sonata/simulations/handlers/SonataReportHandler.cpp
    plugin/io/sonata/simulations/handlers/SonataSpikeHandler.cpp
    plugin/io/sonata/simulations/reports/EdgeCompartmentLoader.cpp
    plugin/io/sonata/simulations/reports/NodeCompartmentLoader.cpp
    plugin/io/sonata/simulations/reports/NodeSpikeLoader.cpp
    plugin/io/sonata/synapse/groups/AggregateGroup.cpp
    plugin/io/sonata/synapse/groups/EndFootGroup.cpp
    plugin/io/sonata/synapse/groups/SurfaceSynapseGroup.cpp
    plugin/io/sonata/synapse/groups/SynapseAstrocyteGroup.cpp
)

set(BRAYNSCIRCUITEXPLORER_PUBLIC_HEADERS
    plugin/api/CellObjectMapper.h
    plugin/api/CircuitExplorerParams.h
    plugin/api/MorphologyMap.h
    plugin/meshing/MetaballsGenerator.h
    plugin/meshing/PointCloudMesher.h
    plugin/CircuitExplorerPlugin.h
    plugin/io/CellGrowthHandler.h
    plugin/io/VoltageSimulationHandler.h
    plugin/io/SpikeSimulationHandler.h
    plugin/io/BrickLoader.h
    plugin/io/AbstractCircuitLoader.h
    plugin/io/PairSynapsesLoader.h
    plugin/io/MeshCircuitLoader.h
    plugin/io/MorphologyCollageLoader.h
    plugin/io/AdvancedCircuitLoader.h
    plugin/io/AstrocyteLoader.h
    plugin/io/SynapseCircuitLoader.h
    plugin/io/MorphologyLoader.h
    plugin/io/SynapseJSONLoader.h
    plugin/io/Utils.h
    plugin/io/sonata/SonataFactory.h
    plugin/io/sonata/SonataLoader.h
    plugin/io/sonata/SonataLoaderEnums.h
    plugin/io/sonata/SonataLoaderProperties.h
    plugin/io/sonata/SonataNGVLoader.h
    plugin/io/sonata/data/SonataCells.h
    plugin/io/sonata/data/SonataSelection.h
    plugin/io/sonata/data/SonataSynapses.h
    plugin/io/sonata/data/SonataVasculature.h
    plugin/io/sonata/morphology/MorphologyInstance.h
    plugin/io/sonata/morphology/neuron/NeuronBuilder.h
    plugin/io/sonata/morphology/neuron/NeuronMorphology.h
    plugin/io/sonata/morphology/neuron/NeuronMorphologyPipeline.h
    plugin/io/sonata/morphology/neuron/builders/PrimitiveNeuronBuilder.h
    plugin/io/sonata/morphology/neuron/builders/SDFNeuronBuilder.h
    plugin/io/sonata/morphology/neuron/builders/SampleNeuronBuilder.h
    plugin/io/sonata/morphology/neuron/instances/PrimitiveNeuronInstance.h
    plugin/io/sonata/morphology/neuron/instances/SDFNeuronInstance.h
    plugin/io/sonata/morphology/neuron/instances/SampleNeuronInstance.h
    plugin/io/sonata/morphology/neuron/pipeline/RadiusMultiplier.h
    plugin/io/sonata/morphology/neuron/pipeline/RadiusSmoother.h
    plugin/io/sonata/morphology/neuron/pipeline/SectionWelder.h
    plugin/io/sonata/morphology/vasculature/VasculatureMorphology.h
    plugin/io/sonata/morphology/vasculature/VasculatureSDFBuilder.h
    plugin/io/sonata/morphology/vasculature/VasculatureSDFInstance.h
    plugin/io/sonata/populations/EdgePopulationLoader.h
    plugin/io/sonata/populations/NodePopulationLoader.h
    plugin/io/sonata/populations/edges/ChemicalSynapsePopulationLoader.h
    plugin/io/sonata/populations/edges/CommonEdgePopulationLoader.h
    plugin/io/sonata/populations/edges/ElectricalSynapsePopulationLoader.h
    plugin/io/sonata/populations/edges/EndFootPopulationLoader.h
    plugin/io/sonata/populations/edges/GlialGlialPopulationLoader.h
    plugin/io/sonata/populations/edges/SynapseAstrocytePopulationLoader.h
    plugin/io/sonata/populations/nodes/AstrocytePopulationLoader.h
    plugin/io/sonata/populations/nodes/BiophysicalPopulationLoader.h
    plugin/io/sonata/populations/nodes/VasculaturePopulationLoader.h
    plugin/io/sonata/simulations/SimulationLoader.h
    plugin/io/sonata/simulations/TransferFunctionSetUp.h
    plugin/io/sonata/simulations/handlers/SonataReportHandler.h
    plugin/io/sonata/simulations/handlers/SonataSpikeHandler.h
    plugin/io/sonata/simulations/reports/EdgeCompartmentLoader.h
    plugin/io/sonata/simulations/reports/NodeCompartmentLoader.h
    plugin/io/sonata/simulations/reports/NodeSpikeLoader.h
    plugin/io/sonata/synapse/SynapseGroup.h
    plugin/io/sonata/synapse/groups/AggregateGroup.h
    plugin/io/sonata/synapse/groups/EndFootGroup.h
    plugin/io/sonata/synapse/groups/SurfaceSynapseGroup.h
    plugin/io/sonata/synapse/groups/SynapseAstrocyteGroup.h
)

set(BRAYNSCIRCUITEXPLORER_ISPC_SOURCES
    module/ispc/camera/utils.ispc
    module/ispc/camera/DOFPerspectiveCamera.ispc
    module/ispc/camera/SphereClippingPerspectiveCamera.ispc
    module/ispc/render/CircuitExplorerMaterial.ispc
    module/ispc/render/CircuitExplorerBasicRenderer.ispc
    module/ispc/render/CircuitExplorerAdvancedRenderer.ispc
    module/ispc/render/VoxelizedSimulationRenderer.ispc
    module/ispc/render/ProximityDetectionRenderer.ispc
    module/ispc/render/CellGrowthRenderer.ispc
    module/ispc/render/utils/CircuitExplorerSimulationRenderer.ispc
    module/ispc/render/utils/CircuitExplorerRandomGenerator.ispc
)

# Compile ispc code
set(BRAYNS_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../..)
list(APPEND ALL_ISPC_INCLUDES ${OSPRAY_INCLUDE_DIRS} ${BRAYNS_SOURCE_DIR})
include_directories_ispc(${ALL_ISPC_INCLUDES} )
ospray_ispc_compile(${BRAYNSCIRCUITEXPLORER_ISPC_SOURCES})
list(APPEND BRAYNSCIRCUITEXPLORER_SOURCES ${ISPC_OBJECTS})

# Compile C++ code
include_directories(
    SYSTEM
    ${CMAKE_CURRENT_SOURCE_DIR}/.
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

set(BRAYNSCIRCUITEXPLORER_LINK_LIBRARIES
    PUBLIC ${OSPRAY_LIBRARIES}
    PRIVATE braynsCommon braynsParameters braynsIO braynsEngine braynsPluginAPI
    braynsOSPRayEngine glm sonata::sonata_shared MorphIO::morphio_shared Brion Brain
    ${FREEIMAGE_LIBRARIES} ${HDF5_LIBRARIES}
)

set(BRAYNSCIRCUITEXPLORER_OMIT_LIBRARY_HEADER ON)
set(BRAYNSCIRCUITEXPLORER_OMIT_VERSION_HEADERS ON)
set(BRAYNSCIRCUITEXPLORER_OMIT_EXPORT ON)

# Movie generation with FFmpeg
find_library(AVFORMAT_LIBRARY avformat)
find_library(AVCODEC_LIBRARY avcodec)
find_library(AVUTIL_LIBRARY avutil)
find_library(SWSCALE_LIBRARY swscale)

set(FFMPEG_FOUND ON)

if(NOT AVFORMAT_LIBRARY)
    message(STATUS "libavformat not found")
    set(FFMPEG_FOUND OFF)
endif()

if(NOT AVCODEC_LIBRARY)
    message(STATUS "libavcodec not found")
    set(FFMPEG_FOUND OFF)
endif()

if(NOT AVUTIL_LIBRARY)
    message(STATUS "libavutil not found")
    set(FFMPEG_FOUND OFF)
endif()

if(NOT SWSCALE_LIBRARY)
    message(STATUS "libswscale not found")
    set(FFMPEG_FOUND OFF)
endif()

if(FFMPEG_FOUND)
    message(STATUS "FFmpeg dev libraries found, enable movie generation")
    list(APPEND BRAYNSCIRCUITEXPLORER_LINK_LIBRARIES
        ${AVFORMAT_LIBRARY}
        ${AVCODEC_LIBRARY}
        ${AVUTIL_LIBRARY}
        ${SWSCALE_LIBRARY}
    )
else()
    message(STATUS "One or more FFmpeg dev libraries not found, disable movie generation")
endif()

list(APPEND BRAYNSCIRCUITEXPLORER_SOURCES plugin/movie/MovieMaker.cpp)

common_library(braynsCircuitExplorer)

if(FFMPEG_FOUND)
    target_compile_definitions(braynsCircuitExplorer PRIVATE BRAYNS_USE_FFMPEG)
endif()
