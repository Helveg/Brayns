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

#include "SonataLoaderProperties.h"

#include <bbp/sonata/node_sets.h>

#include <brayns/common/utils/filesystem.h>
#include <brayns/common/utils/stringUtils.h>

#include <unordered_set>

namespace sonataloader
{
namespace
{
using string_list = std::vector<std::string>;

void __checkEdges(const bbp::sonata::CircuitConfig& config,
                  const brayns::PropertyMap& props,
                  const size_t numNodes)
{
    const auto diskEdgePopulations = config.listEdgePopulations();
    const auto& edgePops = props.getPropertyRef<string_list>(PROPERTY_EDGEPOPULATIONS.name);
    if(edgePops.size() != numNodes)
        throw std::invalid_argument("A comma-separated list of edge populations must be "
                                    "specified, one per population (or an empty string)");

    const auto& edgePercents = props.getPropertyRef<string_list>(PROPERTY_EDGEPERCENTAGES.name);
    if(edgePercents.size() != numNodes)
        throw std::invalid_argument("A comma-separated list of edge percentages must "
                                    "be specified, one per population");

    const auto& edgeModes = props.getPropertyRef<string_list>(PROPERTY_EDGELOADMODES.name);
    if(edgeModes.size() != numNodes)
        throw std::invalid_argument("A comma-separated list of edge load modes must "
                                    "be specified, one per population");

    const auto& edgeSims = props.getPropertyRef<string_list>(PROPERTY_EDGESIMULATIONPATHS.name);
    if(edgeModes.size() != numNodes)
        throw std::invalid_argument("A comma-separated list of edge reports must "
                                    "be specified, one per population (or an empty string)");

    const auto nodePops = props.getPropertyRef<string_list>(PROPERTY_NODEPOPULATIONS.name);
    for(size_t i = 0; i < edgePops.size(); ++i)
    {
        const auto& edgePopList = edgePops[i];
        if(edgePopList.empty())
            continue;

        const auto& edgePercentList = edgePercents[i];
        const auto& edgeModeList = edgeModes[i];
        const auto& edgeSimList = edgeSims[i];
        const auto edgePopTokens = brayns::string_utils::split(edgePopList, ',');
        const auto edgePercentTokens = brayns::string_utils::split(edgePercentList, ',');
        const auto edgeModeTokens = brayns::string_utils::split(edgeModeList, ',');
        const auto edgeSimTokens = brayns::string_utils::split(edgeSimList, ',');

        if(edgePopTokens.size() != edgePercentTokens.size())
            throw std::invalid_argument("Node population '" + nodePops[i] + "': Edge populations "
                                        "and edge percentages must match in size");
        if(edgePopTokens.size() != edgeModeTokens.size())
            throw std::invalid_argument("Node population '" + nodePops[i] + "': Edge populations "
                                        "and edge load modes must match in size");
        if(!edgeSimTokens.empty() && edgePopTokens.size() != edgeSimTokens.size())
            throw std::invalid_argument("Node population '" + nodePops[i] + "': Edge populations "
                                        "and edge simulation paths must match in size, or an "
                                        "empty string must be provided for the edge simulations");

        for(size_t j = 0; j < edgePopTokens.size(); ++j)
        {
            const auto& name = edgePopTokens[j];

            if(diskEdgePopulations.find(name) == diskEdgePopulations.end())
                throw std::invalid_argument("Node population '" + nodePops[i] + "': Edge "
                                            "population '"+ name +"' not found in network");

            const auto& mode = edgeModeTokens[j];
            const bool afferent = (mode == "afferent");
            const bool efferent = (mode == "efferent");
            if(!afferent && !efferent)
                throw std::invalid_argument("Node population '" + nodePops[i] + "': Unrecognized "
                                            "edge load mode '"+ mode +"' (must be 'afferent' or "
                                            "'efferent'");

            const auto edgePopulation = config.getEdgePopulation(name);
            if(efferent && edgePopulation.source() != nodePops[i])
                throw std::invalid_argument("Node population '" + nodePops[i] + "': Edge "
                                            "population '" + name + "' does not have node "
                                            "population '" + nodePops[i] + "' "
                                            "as source node populations");
            else if(afferent && edgePopulation.target() != nodePops[i])
                throw std::invalid_argument("Node population '" + nodePops[i] + "': Edge "
                                            "population '" + name + "' does not have node "
                                            "population '" + nodePops[i] + "' "
                                            "as target node populations");

            if(!edgeSimTokens.empty())
            {
                const auto& edgeSimPath = edgeSimTokens[j];
                if(!fs::exists(edgeSimPath))
                    throw std::invalid_argument("Node population '" + nodePops[i] + "': Edge "
                                                "report " + edgeSimPath + " file not found");
            }
        }
    }
}

/**
 * @brief Checks the sanity of the parameter to configure the loading of node sets
 */
void __checkNodeSets(const bbp::sonata::CircuitConfig& config,
                     const brayns::PropertyMap& props,
                     const size_t numNodes)
{
    const auto& nodeSets = props.getPropertyRef<string_list>(PROPERTY_NODESETS.name);
    // There must be one specified for each population (it can be an empty string)
    if(numNodes != nodeSets.size())
        throw std::invalid_argument("A comma-separated list of nodeset names must be specified, "
                                    "one per population (or an empty string)");

    // Check if the user requested a nodeset for any population
    bool requestedNodeSets = false;
    for(const auto& nst : nodeSets)
        requestedNodeSets = requestedNodeSets || !nst.empty();

    if(requestedNodeSets)
    {
        // No nodesets path in config file
        if(config.getNodeSetsPath().empty())
            throw std::invalid_argument("Circuit configuration does not provide a path"
                                        " to the nodesets file");

        // Cannot find nodesets file in the given path from config
        if(!fs::exists(config.getNodeSetsPath()))
            std::invalid_argument("Cannot find nodesets file '" + config.getNodeSetsPath() + "'");

        // Check if an non-existing node sets name has been requested
        const auto nodeSetFile = bbp::sonata::NodeSets::fromFile(config.getNodeSetsPath());
        const auto diskNodeSets = nodeSetFile.names();
        for(const auto& nodeSetName : nodeSets)
        {
            const auto nameTokens = brayns::string_utils::split(nodeSetName, ',');
            const std::unordered_set<std::string> uniqueNameTokens (nameTokens.begin(),
                                                                    nameTokens.end());
            for(const auto& name : uniqueNameTokens)
            {
                if(diskNodeSets.find(name) != diskNodeSets.end())
                    throw std::invalid_argument("Node set name '" + name + "' "
                                                "not found in nodesets file");
            }
        }
    }
}

void __checkMorphologyParts(const brayns::PropertyMap& props,
                            const size_t numNodes)
{
    const auto& parts = props.getPropertyRef<string_list>(PROPERTY_NEURONPARTS.name);
    if(parts.size() != numNodes)
        throw std::invalid_argument("A bitwise combination of neuron section IDs must be "
                                    "specified, one per node population (or an empty string)");

    const auto& radiusMultiplier = props.getPropertyRef<string_list>(
                                            PROPERTY_RADIUSMULT.name);
    if(radiusMultiplier.size() != numNodes)
        throw std::invalid_argument("A list of morphology radius multiplier must be specified, "
                                    "one per population (must be greather than 0)");
    for(const auto& multiplier : radiusMultiplier)
    {
        if(std::stof(multiplier) <= 0.f)
            throw std::runtime_error("Morphology radius multiplier must be a value above 0");
    }

    const auto& radiusOverride = props.getPropertyRef<string_list>(PROPERTY_RADIUSOVERRIDE.name);
    if(radiusOverride.size() != numNodes)
        throw std::invalid_argument("A list of radius override must be specified, "
                                    "one per population (must be positive, or 0/empty to "
                                    "disable)");
    for(const auto& override : radiusOverride)
    {
        if(std::stof(override) < 0.f)
            throw std::runtime_error("Radius override must be a value greater or equal to 0");
    }

    const auto& loadMode = props.getPropertyRef<string_list>(PROPERTY_NEURONLOADMODE.name);
    if(loadMode.size() != numNodes)
        throw std::invalid_argument("A list of morphology load modes must be specified, "
                                    "one per population (must be one of the possible values)");
}

void __ensureMinimalSections(const NeuronSection section,
                             const VasculatureSection vascSection,
                             const bbp::sonata::PopulationProperties& props)
{
    if((props.type == "biophysical" || props.type == "astrocyte")
            && section == NeuronSection::NONE)
        throw std::invalid_argument("At least a valid neuron morphology seciton must be specified "
                                    "to load for biophysical and astorcyte node populations");
    else if(props.type == "vasculature" && vascSection == VasculatureSection::NONE)
        throw std::invalid_argument("At least a valid vasculature seciton must be specified "
                                    "to load for vasculature node populations");
}

void __checkNodeIds(const brayns::PropertyMap& props,
                    const size_t numNodes)
{
    const auto& nodeIds = props.getPropertyRef<string_list>(PROPERTY_NODEIDS.name);
    if(nodeIds.size() != numNodes)
        throw std::invalid_argument("A comma-separated list of node Ids must be specified "
                                    "for each population (or an empty string to load by "
                                    "percentage / nodesets)");
    for(const auto& popNodeIds : nodeIds)
    {
        string_list partTokens = brayns::string_utils::split(popNodeIds, ',');
        for(const auto& token : partTokens)
        {
            try
            {
                auto test = std::stoull(token);
                (void)test;
            }
            catch(const std::invalid_argument& ia)
            {
                throw std::invalid_argument("Cannot parse node ID '" + token + "'");
            }
        }
    }
}

void __checkSimulation(const brayns::PropertyMap& props,
                       const size_t numNodes)
{
    const auto& simTypes = props.getPropertyRef<string_list>(PROPERTY_NODESIMULATIONTYPE.name);
    if(simTypes.size() != numNodes)
        throw std::invalid_argument("A list of node simulation types must be specified "
                                    "for each population");

    const EnumWrapper<SimulationType> simTypeEnum;
    std::vector<SimulationType> types (simTypes.size());
    for(size_t i = 0; i < simTypes.size(); ++i)
        types[i] = simTypeEnum.fromString(simTypes[i]);

    const auto& simPaths = props.getPropertyRef<string_list>(PROPERTY_NODESIMULATIONFILEPATH.name);
    if(simPaths.size() != numNodes)
        throw std::invalid_argument("A list of node simulation paths must be specified "
                                    "for each population (or an empty string if the repective "
                                    "simulation type is 'none')");
    for(size_t i = 0; i < simPaths.size(); ++i)
    {
        if(types[i] != SimulationType::NONE && (simPaths.empty() || !fs::exists(simPaths[i])))
            throw std::invalid_argument("Cannot find simulation file " + simPaths[i] + " "
                                        "for simulation type " + simTypes[i]);
    }
}

void __checkVasculature(const brayns::PropertyMap& props,
                        const size_t numNodes)
{
    const auto& vascParts = props.getPropertyRef<string_list>(PROPERTY_VASCULATUREPARTS.name);
    if(vascParts.size() != numNodes)
        throw std::invalid_argument("A bitwise combination of vasculature section IDs must be "
                                    "specified, one per node population (or an empty string)");

    const auto& vascRadiiReports =
            props.getPropertyRef<string_list>(PROPERTY_VASCULATURERADIIREPORT.name);
    if(vascRadiiReports.size() != numNodes)
        throw std::invalid_argument("A list of vasculature radii report must be provided, one "
                                    "entry per node population (or an empty string)");
}

/**
 * @brief Check correctness of input loader parameter and disk files
 */
void __checkParameters(const bbp::sonata::CircuitConfig& config,
                       const brayns::PropertyMap& props,
                       const brayns::PropertyMap& defaultProperties)
{
    // CHECK THAT ALL THE PROPERTIES HAVE BEEN PROVIDED
    for(const auto& property : defaultProperties.getProperties())
    {
        if(!props.hasProperty(property->name))
            throw std::invalid_argument("Missing property '" + property->name + "'");
    }

    // CHECK NODE POPULATIONS
    // List of requested node populations cannot be empty
    const auto& nodePops = props.getPropertyRef<string_list>(
                                    PROPERTY_NODEPOPULATIONS.name);
    const size_t numNodePopulations = nodePops.size();

    if(nodePops.empty())
        throw std::invalid_argument("No node populations specified");

    // Check that all requested node populations exists on disk
    const std::unordered_set<std::string> uniqueNodePops (nodePops.begin(), nodePops.end());
    const auto diskNodePops = config.listNodePopulations();
    for(const auto& requestedPopulation : uniqueNodePops)
    {
        if(diskNodePops.find(requestedPopulation) == diskNodePops.end())
            throw std::invalid_argument("Node population '" + requestedPopulation + "'"
                                        + " not found in network");
    }

    // CHECK NODESETS
    __checkNodeSets(config, props, numNodePopulations);
    // CHECK EDGES
    __checkEdges(config, props, numNodePopulations);
    // CHECK MORPHOLOGY PARTS
    __checkMorphologyParts(props, numNodePopulations);
    // CHECK NODE IDS
    __checkNodeIds(props, numNodePopulations);
    // CHECK REPORTS
    __checkSimulation(props, numNodePopulations);
    // CHECK VASCULATURE
    __checkVasculature(props, numNodePopulations);
}

/**
 * Utilities to convert string into different types
 */
template<typename T>
T convert(const std::string& src)
{
    throw std::runtime_error("No known conversion from std::string to "
                             + std::string(typeid(T).name()));
}
template<>
std::string convert(const std::string& src)
{
    return src;
}
template<>
uint64_t convert(const std::string& src)
{
    return static_cast<uint64_t>(std::stoull(src));
}
template<>
float convert(const std::string& src)
{
    return std::stof(src);
}

template<typename T>
std::vector<std::vector<T>> parseStringList(const string_list& list, const char separator = ',')
{
    std::vector<std::vector<T>> result (list.size());
    for(size_t i = 0; i < list.size(); ++i)
    {
        const auto& str = list[i];
        auto tokenList = brayns::string_utils::split(str, separator);
        if(tokenList.empty())
            continue;

        auto& dst = result[i];
        dst.resize(tokenList.size(), T());
        for(size_t j = 0; j < tokenList.size(); ++j)
        {
            if(!tokenList[j].empty())
            {
                brayns::string_utils::trim(tokenList[j]);
                dst[j] = convert<T>(tokenList[j]);
            }
        }
    }
    return result;
}

template<typename T>
std::vector<T> parseFlatStringList(const string_list& list)
{
    std::vector<T> result (list.size(), T());
    for(size_t i = 0; i < list.size(); ++i)
    {
        if(!list[i].empty())
        {
            auto token = list[i];
            brayns::string_utils::trim(token);
            result[i] = convert<T>(token);
        }
    }
    return result;
}

/**
 * @brief synapse_astrocyte edge populations connect a single astorcyte edge population
 *        to one or more edge population of the target neuronal node population. Because
 *        the design is to have a single node or edge population per model, we must check
 *        the requests to load such type of edge populations and populate the neuronal node
 *        populations with the appropiate edge populations. Otherwise, we would need to support
 *        multiple populations per model.
 */
void processAstrocyteSynapseEdges(const bbp::sonata::CircuitConfig& config,
                                  std::vector<PopulationLoadConfig>& loadConfigs)
{
    for(auto& lc : loadConfigs)
    {
        const auto props = config.getNodePopulationProperties(lc.node.name);
        if(props.type != "biophysical")
            continue;

        auto edgeIt = lc.edges.begin();
        std::vector<EdgeLoadConfig> newEdges;
        while(edgeIt != lc.edges.end())
        {
            const auto edgeProps = config.getEdgePopulationProperties((*edgeIt).name);
            if(edgeProps.type == "synapse_astrocyte")
            {
                const auto edges = config.getEdgePopulation((*edgeIt).name);
                const auto neuronEdges = edges.getAttribute<std::string>("synapse_population",
                                                                         edges.selectAll());
                const std::unordered_set<std::string> uniqueNeuronEdges (neuronEdges.begin(),
                                                                         neuronEdges.end());
                for(const auto& neuronEdge : uniqueNeuronEdges)
                    newEdges.push_back({neuronEdge, true, (*edgeIt).percentage, (*edgeIt).report});

                lc.edges.erase(edgeIt);
            }
            else
                ++edgeIt;
        }

        lc.edges.insert(lc.edges.end(), newEdges.begin(), newEdges.end());
    }
}

} // namespace

brayns::PropertyMap SonataLoaderProperties::getPropertyList() noexcept
{
    brayns::PropertyMap props;
    props.setProperty(PROPERTY_NODEPOPULATIONS);
    props.setProperty(PROPERTY_NODESETS);
    props.setProperty(PROPERTY_NODEPERCENTAGE);
    props.setProperty(PROPERTY_NODEIDS);
    props.setProperty(PROPERTY_NODESIMULATIONTYPE);
    props.setProperty(PROPERTY_NODESIMULATIONTYPE);
    props.setProperty(PROPERTY_EDGEPOPULATIONS);
    props.setProperty(PROPERTY_EDGEPERCENTAGES);
    props.setProperty(PROPERTY_EDGELOADMODES);
    props.setProperty(PROPERTY_EDGESIMULATIONPATHS);
    props.setProperty(PROPERTY_RADIUSMULT);
    props.setProperty(PROPERTY_RADIUSOVERRIDE);
    props.setProperty(PROPERTY_NEURONPARTS);
    props.setProperty(PROPERTY_NEURONLOADMODE);
    props.setProperty(PROPERTY_VASCULATUREPARTS);
    props.setProperty(PROPERTY_VASCULATURERADIIREPORT);
    return props;
}

std::vector<PopulationLoadConfig>
SonataLoaderProperties::checkAndParse(const std::string& path,
                                      const bbp::sonata::CircuitConfig& config,
                                      const brayns::PropertyMap& properties)
{
    // Fast input check to avoid start loading process if problems are detected
    __checkParameters(config, properties, getPropertyList());

    const auto& populationList =
            properties.getPropertyRef<string_list>(PROPERTY_NODEPOPULATIONS.name);

    const auto& nodeSetsRaw =
            properties.getPropertyRef<string_list>(PROPERTY_NODESETS.name);
    const auto nodeSets = parseStringList<std::string>(nodeSetsRaw);

    const auto& nodeLoadPercentagesRaw =
            properties.getPropertyRef<string_list>(PROPERTY_NODEPERCENTAGE.name);
    const auto nodeLoadPercentages = parseFlatStringList<float>(nodeLoadPercentagesRaw);

    const auto& nodeIdsRaw =
            properties.getPropertyRef<string_list>(PROPERTY_NODEIDS.name);
    const auto nodeIds = parseStringList<uint64_t>(nodeIdsRaw);

    const auto& simTypesRaw =
            properties.getPropertyRef<string_list>(PROPERTY_NODESIMULATIONTYPE.name);
    const auto simTypes = parseFlatStringList<std::string>(simTypesRaw);

    const auto simPaths =
            properties.getPropertyRef<string_list>(PROPERTY_NODESIMULATIONFILEPATH.name);

    const auto& edgePopulationsRaw =
            properties.getPropertyRef<string_list>(PROPERTY_EDGEPOPULATIONS.name);
    const auto edgePopulations = parseStringList<std::string>(edgePopulationsRaw);

    const auto& edgePercentagesRaw =
            properties.getPropertyRef<string_list>(PROPERTY_EDGEPERCENTAGES.name);
    const auto edgePercentages = parseStringList<float>(edgePercentagesRaw);

    const auto& edgeLoadModesRaw =
            properties.getPropertyRef<string_list>(PROPERTY_EDGELOADMODES.name);
    const auto edgeLoadModes = parseStringList<std::string>(edgeLoadModesRaw);

    const auto& edgeSimsRaw =
            properties.getPropertyRef<string_list>(PROPERTY_EDGESIMULATIONPATHS.name);
    const auto edgeSims = parseStringList<std::string>(edgeSimsRaw);

    const auto& morphologySectionsRaw =
            properties.getPropertyRef<string_list>(PROPERTY_NEURONPARTS.name);
    const auto morphologySections = parseStringList<std::string>(morphologySectionsRaw);

    const auto& morphologyRadiusMultRaw =
            properties.getPropertyRef<string_list>(PROPERTY_RADIUSMULT.name);
    const auto morphologyRadiusMult = parseFlatStringList<float>(morphologyRadiusMultRaw);

    const auto& radiusOverrideRaw =
            properties.getPropertyRef<string_list>(PROPERTY_RADIUSOVERRIDE.name);
    const auto raidusOverride = parseFlatStringList<float>(radiusOverrideRaw);

    const auto& morphologyLoadMode =
            properties.getPropertyRef<string_list>(PROPERTY_NEURONLOADMODE.name);

    const auto& vasculatureSectionsRaw =
            properties.getPropertyRef<string_list>(PROPERTY_VASCULATUREPARTS.name);
    const auto vasculatureSections = parseStringList<std::string>(vasculatureSectionsRaw);

    const auto& vasculatureRadiiReports =
            properties.getPropertyRef<string_list>(PROPERTY_VASCULATURERADIIREPORT.name);

    std::vector<PopulationLoadConfig> populations(populationList.size());
    for(size_t i = 0; i < populationList.size(); ++i)
    {
        const bbp::sonata::PopulationProperties props =
                config.getNodePopulationProperties(populationList[i]);

        auto& pls = populations[i];
        pls.configPath = path;
        pls.node.name = populationList[i];

        pls.node.ids = nodeIds[i];
        if(!pls.node.ids.empty())
            std::sort(pls.node.ids.begin(), pls.node.ids.end());

        pls.node.nodeSets = nodeSets[i];
        pls.node.percentage = glm::clamp(nodeLoadPercentages[i], 0.f, 1.f);
        if(pls.node.percentage == 0.f)
            throw std::invalid_argument("SonataLoader: A negative or 0 node percentage "
                                        "is not allowed");

        pls.node.simulationType = EnumWrapper<SimulationType>().fromString((simTypes[i]));
        pls.node.simulationPath = simPaths[i];

        if(!edgePopulations.empty())
        {
            pls.edges.resize(edgePopulations[i].size());
            for(size_t j = 0; j < edgePopulations[i].size(); ++j)
            {
                pls.edges[j].name = edgePopulations[i][j];
                pls.edges[j].percentage = edgePercentages[i][j];
                pls.edges[j].afferent = edgeLoadModes[i][j] == "afferent";
                if(!edgeSims[i].empty())
                    pls.edges[j].report = edgeSims[i][j];
            }
        }

        pls.neurons.radiusMultiplier = morphologyRadiusMult[i];
        pls.vasculature.radiusMultiplier = morphologyRadiusMult[i];

        if(props.type != "vasculature")
        {
            pls.neurons.sections = NeuronSection::NONE;
            if(!morphologySections.empty())
            {
                const EnumWrapper<NeuronSection> neuronSections;
                for(const auto& part : morphologySections[i])
                    pls.neurons.sections |= neuronSections.fromString(part);
            }

            // If only soma requested, use primitive geometry
            const EnumWrapper<NeuronGeometryType> geomTypeEnum;
            pls.neurons.mode = pls.neurons.sections == NeuronSection::SOMA?
                        NeuronGeometryType::SAMPLES : geomTypeEnum.fromString(morphologyLoadMode[i]);
        }
        else
        {
            pls.vasculature.sections = VasculatureSection::NONE;
            if(!vasculatureSections.empty())
            {
                const EnumWrapper<VasculatureSection> vasculatureSection;
                for(const auto& part : vasculatureSections[i])
                    pls.vasculature.sections |= vasculatureSection.fromString(part);
            }
        }

        // Make sure enough info has been specified to avoid loading an empty model
        __ensureMinimalSections(pls.neurons.sections,
                                pls.vasculature.sections,
                                props);

        pls.vasculature.radiiReport = vasculatureRadiiReports[i];
    }

    processAstrocyteSynapseEdges(config, populations);
    return populations;
}
}
