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
                        const PopulationLoadConfig& loadConfig)
{
    NodeSelection selection;
    selection.select(config, loadConfig.name, loadConfig.nodeSets);
    selection.select(loadConfig.nodeIds);
    selection.select(loadConfig.simulationType, loadConfig.simulationPath, loadConfig.name);
    return selection.intersection(loadConfig.percentage);
}

inline NodeSimulationLoaderPtr
instantiateNodeSimulation(const SonataFactories& factories, const PopulationLoadConfig& config)
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
                 const PopulationLoadConfig& loadConfig)
{
    auto properties = circuitConfig.getNodePopulationProperties(loadConfig.name);
    return factories.nodeLoaders().instantiate(properties.type,
                                               circuitConfig.getNodePopulation(loadConfig.name),
                                               std::move(properties));
}

inline std::unique_ptr<EdgePopulationLoader>
instantiateEdges(const SonataFactories& factories,
                 const bbp::sonata::CircuitConfig& circuitConfig,
                 const std::string& edgePopulation,
                 const float percentage)
{
    const auto type = circuitConfig.getEdgePopulationProperties(edgePopulation).type;
    return factories.edgeLoaders().instantiate(type,
                                               circuitConfig,
                                               edgePopulation,
                                               percentage);
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


SonataLoader::SonataLoader(brayns::Scene& scene)
 : brayns::Loader(scene)
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

    for(size_t i = 0; i < requestedPopulations.size(); ++i)
    {
        const auto& loadConfig = requestedPopulations[i];
        const auto nodeSelection = selectNodes(config, loadConfig);
        const auto nodeSize = nodeSelection.flatSize();

        if(nodeSelection.empty())
            throw std::runtime_error("Population " + loadConfig.name + " node selection is empty");

        // LOAD NODES
        const auto nodeLoader  = instantiateNodes(factories, config, loadConfig);
        auto nodes = nodeLoader->load(loadConfig, nodeSelection, callback);

        const auto simulation = instantiateNodeSimulation(factories, loadConfig);
        if(simulation)
        {
            const auto mapping = simulation->loadMapping(nodeSelection);
            for(size_t i = 0; i < nodes.size(); ++i)
            {
                const auto& cm = mapping[i];
                nodes[i]->mapSimulation(cm.globalOffset, cm.offsets, cm.compartments);
            }
        }

        brayns::ModelPtr nodeMmodel = _scene.createModel();
        for(const auto& node : nodes)
            node->addToModel(*nodeMmodel);
        nodeMmodel->updateBounds();
        if(simulation)
        {
            nodeMmodel->setSimulationHandler(simulation->createSimulationHandler(nodeSelection));
            SetSONATATransferFunction(_scene.getTransferFunction());
        }

        brayns::ModelMetadata nodeMetadata = {
            {"Population", loadConfig.name},
            {"Type", config.getNodePopulationProperties(loadConfig.name).type},
            {"Report", loadConfig.simulationPath},
            {"Node Sets", brayns::string_utils::join(loadConfig.nodeSets, ",")},
            {"Number of nodes", std::to_string(nodeSelection.flatSize())},
            {"Circuit Path", path}
        };

        result.push_back(createModelDescriptor(loadConfig.name, path, nodeMetadata, nodeMmodel));
        PLUGIN_INFO << "Loaded node population " << loadConfig.name << std::endl;

        // LOAD EDGES
        for(size_t i = 0; i < loadConfig.edgePopulations.size(); ++i)
        {
            const auto& edgePop = loadConfig.edgePopulations[i];

            const auto edgePerc = loadConfig.edgePercentages[i];
            const auto& edgeMode = loadConfig.edgeLoadModes[i];

            const auto edgeLoader = instantiateEdges(factories, config, edgePop, edgePerc);
            const auto edges = edgeLoader->load(loadConfig, nodeSelection, edgeMode == "afferent");
            if(edges.empty())
                continue;

            brayns::ModelPtr edgeModel = _scene.createModel();

            for(size_t j = 0; j < nodes.size(); ++j)
                edges[j]->mapToCell(*nodes[j]);

            std::string edgeReport = "";
            if(!loadConfig.edgeReports.empty() && !loadConfig.edgeReports[i].empty())
            {
                edgeReport = loadConfig.edgeReports[i];
                const EdgeCompartmentLoader loader (loadConfig.edgeReports[i], edgePop);
                const auto mapping = loader.loadMapping(nodeSelection);
                for(size_t j = 0; j < edges.size(); ++j)
                    edges[i]->mapSimulation(mapping[j].offsets);
            }

            for(size_t j = 0; j < nodes.size(); ++j)
                edges[j]->addToModel(*edgeModel);

            brayns::ModelMetadata edgeMetadata = {
                {"Population", edgePop},
                {"Type", config.getEdgePopulationProperties(edgePop).type},
                {"Report", edgeReport},
                {"Circuit Path", path}
            };

            result.push_back(createModelDescriptor(edgePop, path, edgeMetadata, edgeModel));

            PLUGIN_INFO << "Loaded " <<edgePop<< " for " <<loadConfig.name<< " nodes" << std::endl;
        }
    }
    return result;
}
