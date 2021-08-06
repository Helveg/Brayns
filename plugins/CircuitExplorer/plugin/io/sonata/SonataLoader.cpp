/* Copyright (c) 2015-2021, EPFL/Blue Brain Project
 * All rights reserved. Do not distribute without permission.
 * Responsible Author: Nadir Roman <nadir.romanguerrero@epfl.ch>
 *
 * This file is part of the circuit explorer for Brayns
 * <https://github.com/favreau/Brayns-UC-CircuitExplorer>
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

#include "SonataLoaderProperties.h"
#include "data/SonataSelection.h"
#include "populations/PopulationFactory.h"
#include "simulations/SonataSimulationFactory.h"
#include "simulations/TransferFunctionSetUp.h"

#include "../../../common/log.h"

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

inline std::unique_ptr<SonataSimulation> instantiateSimulation(const PopulationLoadConfig& config)
{
    if(config.simulationType == SimulationType::NONE)
        return {nullptr};

    return SonataSimulationFactory::instance().createSimulation(config.simulationType,
                                                                config.simulationPath,
                                                                config.name);
}

inline std::unique_ptr<NodePopulationLoader>
instantiateNodes(const bbp::sonata::CircuitConfig& circuitConfig,
                 const PopulationLoadConfig& loadConfig)
{
    return PopulationFactory::instance()
                    .createNodeLoader(circuitConfig.getNodePopulation(loadConfig.name),
                                      circuitConfig.getNodePopulationProperties(loadConfig.name));
}

inline std::unique_ptr<EdgePopulationLoader>
instantiateEdges(const bbp::sonata::CircuitConfig& circuitConfig,
                 const PopulationLoadConfig& loadConfig,
                 const std::string& edgePopulation)
{
    bbp::sonata::EdgePopulation edges = circuitConfig.getEdgePopulation(edgePopulation);
    if(edges.target() != loadConfig.name && edges.source() != loadConfig.name)
        throw std::runtime_error("Requested edge population '" + edgePopulation
                                 + "' does not have node population '" + loadConfig.name
                                 + "' neither as source or target node population");

    return PopulationFactory::instance()
                    .createEdgeLoader(std::move(edges),
                                      circuitConfig.getEdgePopulationProperties(edgePopulation));
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

    const bbp::sonata::CircuitConfig config = bbp::sonata::CircuitConfig::fromFile(path);

    // Check input loading paraemters <-> disk files sanity
    const SonataLoaderProperties loaderProps (config, props);
    const auto& requestedPopulations = loaderProps.getRequestedPopulations();

    // Load each population
    std::vector<brayns::ModelDescriptorPtr> result (requestedPopulations.size());

    for(size_t i = 0; i < requestedPopulations.size(); ++i)
    {
        // User load settings for this population
        const auto& loadConfig = requestedPopulations[i];

        // Compute the node IDs that we will load
        const auto nodeSelection = selectNodes(config, loadConfig);

        if(nodeSelection.empty())
            throw std::runtime_error("Population " + loadConfig.name + " node selection is empty");

        // Load nodes
        const auto nodeLoader  = instantiateNodes(config, loadConfig);
        auto nodes = nodeLoader->load(loadConfig, nodeSelection, callback);

        // Load edges (synapses) into the nodes
        for(const auto& afferent : loadConfig.afferentPopulations)
        {
            const auto edgeLoader = instantiateEdges(config, loadConfig, afferent);
            const auto synapses = edgeLoader->load(loadConfig, nodeSelection, true);
            for(size_t i = 0; i < nodes.size(); ++i)
            {
                auto& node = nodes[i];
                for(const auto& s : synapses[i])
                    node->addSynapse(s.synapseId, s.sectionId, s.distance, s.position);
            }
        }
        for(const auto& efferent : loadConfig.efferentPopulations)
        {
            const auto edgeLoader = instantiateEdges(config, loadConfig, efferent);
            const auto synapses = edgeLoader->load(loadConfig, nodeSelection, false);
            for(size_t i = 0; i < nodes.size(); ++i)
            {
                auto& node = nodes[i];
                for(const auto& s : synapses[i])
                    node->addSynapse(s.synapseId, s.sectionId, s.distance, s.position);
            }
        }

        // Map simulation
        const auto simulation = instantiateSimulation(loadConfig);
        if(simulation)
        {
            const auto mapping = simulation->loadMapping(nodeSelection);
            for(size_t i = 0; i < nodes.size(); ++i)
            {
                const auto& cm = mapping[i];
                nodes[i]->mapSimulation(cm.globalOffset, cm.offsets, cm.compartments);
            }
        }

        // Add to brayns model
        brayns::ModelPtr model = _scene.createModel();
        for(const auto& node : nodes)
            node->addToModel(*model);
        model->updateBounds();

        // Add simulation handler
        if(simulation)
        {
            model->setSimulationHandler(simulation->createSimulationHandler(nodeSelection));
            SetSONATATransferFunction(_scene.getTransferFunction());
        }

        // Create descriptor
        brayns::ModelMetadata metadata = {
            {"Report", loadConfig.simulationPath},
            {"Node Sets", brayns::string_utils::join(loadConfig.nodeSets, ",")},
            {"Number of neurons", std::to_string(nodeSelection.flatSize())},
            {"Circuit Path", path}
        };

        brayns::ModelDescriptorPtr& modelDescriptor = result[i];
        brayns::Transformation transformation;
        transformation.setRotationCenter(model->getBounds().getCenter());
        modelDescriptor =
            std::make_shared<brayns::ModelDescriptor>(std::move(model), "Circuit",
                                                      path,
                                                      metadata);
        modelDescriptor->setTransformation(transformation);
    }

    return result;
}
