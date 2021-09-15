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

#include <brayns/common/Timer.h>
#include <brayns/common/utils/stringUtils.h>
#include <brayns/engine/Scene.h>

#include <plugin/api/Log.h>
#include <plugin/api/MaterialUtils.h>
#include <plugin/io/util/ProgressReport.h>
#include <plugin/io/sonataloader/SonataLoaderProperties.h>
#include <plugin/io/sonataloader/data/SonataSelection.h>
#include <plugin/io/sonataloader/SonataFactory.h>
#include <plugin/io/util/TransferFunctionUtils.h>
#include <plugin/io/sonataloader/simulations/reports/EdgeCompartmentLoader.h>

using namespace sonataloader;

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


SonataLoader::SonataLoader(brayns::Scene& scene,
                           CircuitColorManager& colorManager,
                           VasculatureRadiiSimulation& radiiHandler)
 : brayns::Loader(scene)
 , _colorManager(colorManager)
 , _radiiSimulationHandler(radiiHandler)
{
    PLUGIN_INFO << "Registering loader: " << getName() << std::endl;
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
    return std::string ("SONATA loader");
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
    brayns::Timer timer;
    PLUGIN_INFO << getName() << ": Loading " << path << std::endl;

    const SonataFactories factories;
    const bbp::sonata::CircuitConfig config = bbp::sonata::CircuitConfig::fromFile(path);

    // Check input loading paraemters <-> disk files sanity
    const auto requestedPopulations = SonataLoaderProperties::checkAndParse(path, config, props);

    // Compute how much progress percentage each population load will consume
    size_t numModels = 0u;
    for(const auto& popLoadConfig : requestedPopulations)
        numModels += popLoadConfig.edges.size() + 1;
    float total = 0.f;
    const float chunk = (1.f / static_cast<float>(numModels));

    // Load each population
    std::vector<brayns::ModelDescriptorPtr> result;

    for(const auto& loadConfig : requestedPopulations)
    {
        const auto& node = loadConfig.node;
        const auto nodeSelection = selectNodes(config, node);

        if(nodeSelection.empty())
            throw std::runtime_error("Node population " + node.name + " selection is empty");

        // ---------------------------------------------------------------------------------
        // LOAD NODES
        // ---------------------------------------------------------------------------------
        ProgressReport np (callback, total, chunk, 5u);

        const auto nodeIDs = nodeSelection.flatten();
        const auto nodeLoader  = instantiateNodes(factories, config, node);
        auto nlp = np.nextSubProgress("Loading " + node.name, nodeIDs.size());
        auto nodes = nodeLoader->load(loadConfig, nodeSelection, nlp);
        if(nodes.empty())
            continue;

        brayns::ModelPtr nodeModel = _scene.createModel();

        // Attach simulation, if any
        // ---------------------------------------------------------------------------------
        const auto simulation = instantiateNodeSimulation(factories, node);
        if(simulation)
        {
            auto slp = np.nextSubProgress(node.name + ": Loading simulation", nodes.size());
            const auto mapping = simulation->loadMapping(nodeSelection);
            for(size_t i = 0; i < nodes.size(); ++i)
            {
                const auto& cm = mapping[i];
                nodes[i]->mapSimulation(cm.globalOffset, cm.offsets, cm.compartments);
                slp.tick();
            }
            nodeModel->setSimulationHandler(simulation->createSimulationHandler(nodeSelection));
            TransferFunctionUtils::set(_scene.getTransferFunction());
        }

        // Add geometry to model and get the material mapping
        // ---------------------------------------------------------------------------------
        std::vector<ElementMaterialMap::Ptr> materialMaps;
        materialMaps.reserve(nodes.size());
        auto glp = np.nextSubProgress(node.name + ": Generating geometry", nodes.size());
        for(const auto& n : nodes)
        {
            materialMaps.push_back(n->addToModel(*nodeModel));
            glp.tick();
        }
        if(simulation)
            CircuitExplorerMaterial::setSimulationColorEnabled(*nodeModel, true);

        // Create the model descriptor
        // ---------------------------------------------------------------------------------
        np.nextSubProgress(node.name + ": Generating model");
        nodeModel->updateBounds();
        brayns::ModelMetadata nodeMetadata = {
            {"Population", node.name},
            {"Type", config.getNodePopulationProperties(node.name).type},
            {"Report", node.simulationPath},
            {"Node Sets", brayns::string_utils::join(node.nodeSets, ",")},
            {"Number of nodes", std::to_string(nodeIDs.size())},
            {"Circuit Path", path}
        };
        result.push_back(createModelDescriptor(node.name, path, nodeMetadata, nodeModel));
        auto nodeModelPtr = result.back().get();
        nodeModelPtr->setName(node.name);

        // Create the color handler
        // ---------------------------------------------------------------------------------
        np.nextSubProgress(node.name + ": Generating color mapping");
        auto nodeColorHandler = nodeLoader->createColorHandler(nodeModelPtr, path);
        nodeColorHandler->setElements(nodeIDs, std::move(materialMaps));
        _colorManager.registerHandler(std::move(nodeColorHandler));

        // Handle the special case of vasculature radii report
        // TODO: After engine refactoring, this should not be necessary
        if(simulation && loadConfig.node.simulationType == SimulationType::BLOODFLOW_RADII)
            _radiiSimulationHandler.registerModel(nodeModelPtr);

        nodeModelPtr->onRemoved([cmPtr = &_colorManager,
                                 rPtr = &_radiiSimulationHandler](const brayns::ModelDescriptor& m)
        {
            cmPtr->unregisterHandler(m.getModelID());
            rPtr->unregisterModel(m.getModelID());
        });

        total += chunk;
        PLUGIN_INFO << "Loaded node population " << node.name << std::endl;

        // ---------------------------------------------------------------------------------
        // LOAD EDGES
        // ---------------------------------------------------------------------------------
        for(const auto& edge : loadConfig.edges)
        {
            ProgressReport ep (callback, total, chunk, !edge.report.empty()? 6u : 5u);

            auto elp = ep.nextSubProgress("Loading " + edge.name, nodes.size());
            const auto edgeLoader = instantiateEdges(factories, config, edge);
            const auto edges = edgeLoader->load(loadConfig, nodeSelection, elp);
            if(edges.empty())
            {
                PLUGIN_WARN << "Edge population " << edge.name << " is empty" << std::endl;
                continue;
            }

            brayns::ModelPtr edgeModel = _scene.createModel();

            // Map to the node geometry to which they belong
            // ---------------------------------------------------------------------------------
            auto mlp = ep.nextSubProgress(edge.name + ": Map to node geometry", nodes.size());
            for(size_t j = 0; j < nodes.size(); ++j)
            {
                edges[j]->mapToCell(*nodes[j]);
                mlp.tick();
            }

            // Attach simulation, if any
            // ---------------------------------------------------------------------------------
            if(!edge.report.empty())
            {
                auto eslp = ep.nextSubProgress(edge.name + ": Loading simulation", nodes.size());
                const EdgeCompartmentLoader edgeSim (edge.report, edge.name);
                const auto mapping = edgeSim.loadMapping(nodeSelection);
                for(size_t j = 0; j < nodes.size(); ++j)
                {
                    edges[j]->mapSimulation(mapping[j].offsets);
                    eslp.tick();
                }
                edgeModel->setSimulationHandler(edgeSim.createSimulationHandler(nodeSelection));
                TransferFunctionUtils::set(_scene.getTransferFunction());
            }

            // Add geometry to model and get the material mapping
            // ---------------------------------------------------------------------------------
            std::vector<ElementMaterialMap::Ptr> edgeMaterialMaps (nodes.size());
            auto eglp = ep.nextSubProgress(edge.name + ": Generating geometry", nodes.size());
            for(size_t j = 0; j < nodes.size(); ++j)
            {
                edgeMaterialMaps[j] = edges[j]->addToModel(*edgeModel);
                eglp.tick();
            }
            if(!edge.report.empty())
                CircuitExplorerMaterial::setSimulationColorEnabled(*edgeModel, true);

            // Create the model descriptor
            // ---------------------------------------------------------------------------------
            ep.nextSubProgress(edge.name + ": Generating model");
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
            // ---------------------------------------------------------------------------------
            ep.nextSubProgress(edge.name + ": Generating color mapping");
            auto edgeColorHandler = edgeLoader->createColorHandler(edgeModelPtr, path);
            edgeColorHandler->setElements(nodeIDs, std::move(edgeMaterialMaps));
            edgeModelPtr->onRemoved([cmPtr = &_colorManager](const brayns::ModelDescriptor& m)
            {
                cmPtr->unregisterHandler(m.getModelID());
            });
            _colorManager.registerHandler(std::move(edgeColorHandler));

            total += chunk;
            PLUGIN_INFO << "Loaded " <<edge.name<< " for " <<node.name<< " nodes" << std::endl;
        }
    }

    PLUGIN_INFO << getName() << ": Done in " << timer.elapsed() << " second(s)" << std::endl;

    return result;
}
