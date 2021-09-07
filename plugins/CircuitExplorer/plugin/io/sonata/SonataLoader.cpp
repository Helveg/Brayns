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

#include "SonataLoader.h"

#include <plugin/io/sonata/SonataLoaderProperties.h>
#include <plugin/io/sonata/data/SonataSelection.h>
#include <plugin/io/sonata/SonataFactory.h>
#include <plugin/io/sonata/simulations/TransferFunctionSetUp.h>
#include <plugin/io/sonata/simulations/reports/EdgeCompartmentLoader.h>

#include <common/log.h>

#include <brayns/common/utils/stringUtils.h>
#include <brayns/engine/Scene.h>

namespace
{
inline auto selectNodes(const bbp::sonata::CircuitConfig& config,
                        const NodeLoadConfig& loadConfig)
{
    NodeSelection selection;
    selection.select(config, loadConfig.name, loadConfig.nodeSets);
    selection.select(loadConfig.ids);
    selection.select(loadConfig.simulationType, loadConfig.simulationPath, loadConfig.name);
    return selection.intersection(loadConfig.percentage);
}

inline NodeSimulationLoaderPtr
instantiateNodeSimulation(const SonataFactories& factories, const NodeLoadConfig& config)
{
    if(config.simulationType == SimulationType::NONE)
        return {nullptr};

    return factories.simulations().instantiate(config.simulationType,
                                               config.simulationPath,
                                               config.name);
}

inline std::unique_ptr<NodePopulationLoader>
instantiateNodes(const SonataFactories& factories,
                 const bbp::sonata::CircuitConfig& circuitConfig,
                 const NodeLoadConfig& loadConfig)
{
    auto properties = circuitConfig.getNodePopulationProperties(loadConfig.name);
    return factories.nodeLoaders().instantiate(properties.type,
                                               circuitConfig.getNodePopulation(loadConfig.name),
                                               std::move(properties));
}

inline std::unique_ptr<EdgePopulationLoader>
instantiateEdges(const SonataFactories& factories,
                 const bbp::sonata::CircuitConfig& circuitConfig,
                 const EdgeLoadConfig& config)
{
    const auto type = circuitConfig.getEdgePopulationProperties(config.name).type;
    return factories.edgeLoaders().instantiate(type, circuitConfig, config.name,
                                               config.percentage, config.afferent);
}

inline brayns::ModelDescriptorPtr createModelDescriptor(const std::string& name,
                                                        const std::string& path,
                                                        const brayns::ModelMetadata& metadata,
                                                        brayns::ModelPtr& model)
{
    brayns::Transformation transform;
    transform.setRotationCenter(model->getBounds().getCenter());
    auto modelDescriptor =
        std::make_shared<brayns::ModelDescriptor>(std::move(model), name,
                                                  path,
                                                  metadata);
    modelDescriptor->setTransformation(transform);
    return modelDescriptor;
}
} // namespace


SonataLoader::SonataLoader(brayns::Scene& scene, CircuitColorManager& colorManager)
 : brayns::Loader(scene)
 , _colorManager(colorManager)
{
    PLUGIN_INFO << "Registering " << getName() << std::endl;
}

std::vector<std::string> SonataLoader::getSupportedExtensions() const
{
    return std::vector<std::string> {".json"};
}

bool SonataLoader::isSupported(const std::string&, const std::string& extension) const
{
    return brayns::string_utils::toLowercase(extension) == "json";
}

std::string SonataLoader::getName() const
{
    return std::string ("Sonata circuit loader");
}

brayns::PropertyMap SonataLoader::getProperties() const
{
    return SonataLoaderProperties::getPropertyList();
}

std::vector<brayns::ModelDescriptorPtr>
SonataLoader::importFromBlob(brayns::Blob&& blob,
                             const brayns::LoaderProgress& callback,
                             const brayns::PropertyMap& properties) const
{
    throw std::runtime_error("Sonata loader: import from blob not supported");
}

std::vector<brayns::ModelDescriptorPtr>
SonataLoader::importFromFile(const std::string& path,
                             const brayns::LoaderProgress& callback,
                             const brayns::PropertyMap& props) const
{
    PLUGIN_INFO << "SONATA loader: Importing " << path << std::endl;

    const SonataFactories factories;
    const bbp::sonata::CircuitConfig config = bbp::sonata::CircuitConfig::fromFile(path);

    // Check input loading paraemters <-> disk files sanity
    const SonataLoaderProperties loaderProps (path, config, props);
    const auto& requestedPopulations = loaderProps.getRequestedPopulations();

    // Load each population
    std::vector<brayns::ModelDescriptorPtr> result;

    for(const auto& loadConfig : requestedPopulations)
    {
        const auto& node = loadConfig.node;
        const auto nodeSelection = selectNodes(config, node);

        if(nodeSelection.empty())
            throw std::runtime_error("Population "+node.name+" node selection is empty");

        // ---------------------------------------------------------------------------------
        // LOAD NODES
        // ---------------------------------------------------------------------------------
        const auto nodeIDs = nodeSelection.flatten();
        const auto nodeLoader  = instantiateNodes(factories, config, node);
        auto nodes = nodeLoader->load(loadConfig, nodeSelection, callback);
        if(nodes.empty())
            continue;

        brayns::ModelPtr nodeMmodel = _scene.createModel();

        // Attach simulation, if any
        const auto simulation = instantiateNodeSimulation(factories, node);
        if(simulation)
        {
            const auto mapping = simulation->loadMapping(nodeSelection);
            for(size_t i = 0; i < nodes.size(); ++i)
            {
                const auto& cm = mapping[i];
                nodes[i]->mapSimulation(cm.globalOffset, cm.offsets, cm.compartments);
            }
            nodeMmodel->setSimulationHandler(simulation->createSimulationHandler(nodeSelection));
        }

        // Add geometry to model and get the material mapping
        std::vector<ElementMaterialMap::Ptr> materialMaps;
        materialMaps.reserve(nodes.size());
        for(const auto& node : nodes)
            materialMaps.push_back(node->addToModel(*nodeMmodel));
        nodeMmodel->updateBounds();

        // Create the model descriptor
        brayns::ModelMetadata nodeMetadata = {
            {"Population", node.name},
            {"Type", config.getNodePopulationProperties(node.name).type},
            {"Report", node.simulationPath},
            {"Node Sets", brayns::string_utils::join(node.nodeSets, ",")},
            {"Number of nodes", std::to_string(nodeIDs.size())},
            {"Circuit Path", path}
        };
        result.push_back(createModelDescriptor(node.name, path, nodeMetadata, nodeMmodel));
        auto nodeModelPtr = result.back().get();
        nodeModelPtr->setName(node.name);

        // Create the color handler
        auto nodeColorHandler = nodeLoader->createColorHandler(nodeModelPtr, path);
        nodeColorHandler->setElements(nodeIDs, std::move(materialMaps));
        nodeModelPtr->onRemoved([cmPtr = &_colorManager](const brayns::ModelDescriptor& m)
        {
            cmPtr->unregisterHandler(m.getModelID());
        });
        _colorManager.registerHandler(std::move(nodeColorHandler));


        PLUGIN_INFO << "Loaded node population " << node.name << std::endl;

        // ---------------------------------------------------------------------------------
        // LOAD EDGES
        // ---------------------------------------------------------------------------------
        for(const auto& edge : loadConfig.edges)
        {
            const auto edgeLoader = instantiateEdges(factories, config, edge);
            const auto edges = edgeLoader->load(loadConfig, nodeSelection);
            if(edges.empty())
                continue;

            // Map to the node geometry to which they belong
            for(size_t j = 0; j < nodes.size(); ++j)
                edges[j]->mapToCell(*nodes[j]);

            // Attach simulation, if any
            if(!edge.report.empty())
            {
                const EdgeCompartmentLoader loader (edge.report, edge.name);
                const auto mapping = loader.loadMapping(nodeSelection);
                for(size_t j = 0; j < nodes.size(); ++j)
                    edges[j]->mapSimulation(mapping[j].offsets);
            }

            // Add geometry to model and get the material mapping
            brayns::ModelPtr edgeModel = _scene.createModel();
            std::vector<ElementMaterialMap::Ptr> edgeMaterialMaps (nodes.size());
            for(size_t j = 0; j < nodes.size(); ++j)
                edgeMaterialMaps[j] = edges[j]->addToModel(*edgeModel);

            // Create the model descriptor
            brayns::ModelMetadata edgeMetadata = {
                {"Population", edge.name},
                {"Type", config.getEdgePopulationProperties(edge.name).type},
                {"Report", edge.report},
                {"Circuit Path", path}
            };
            result.push_back(createModelDescriptor(edge.name, path, edgeMetadata, edgeModel));
            auto edgeModelPtr = result.back().get();
            edgeModelPtr->setName(edge.name);

            // Create the color handler
            auto edgeColorHandler = edgeLoader->createColorHandler(edgeModelPtr, path);
            edgeColorHandler->setElements(nodeIDs, std::move(edgeMaterialMaps));
            edgeModelPtr->onRemoved([cmPtr = &_colorManager](const brayns::ModelDescriptor& m)
            {
                cmPtr->unregisterHandler(m.getModelID());
            });
            _colorManager.registerHandler(std::move(edgeColorHandler));

            PLUGIN_INFO << "Loaded " <<edge.name<< " for " <<node.name<< " nodes" << std::endl;
        }
    }

    SetSONATATransferFunction(_scene.getTransferFunction());

    return result;
}
