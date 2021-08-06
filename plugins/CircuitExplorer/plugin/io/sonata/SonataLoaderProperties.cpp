#include "SonataLoaderProperties.h"

#include <brayns/common/utils/stringUtils.h>

#include <bbp/sonata/node_sets.h>

#include <boost/filesystem.hpp>

namespace
{
using string_list = std::vector<std::string>;

/**
 * @brief Checks the sanity of the parameter to configure the loading of edge populations
 */
void checkEdgePopulation(const bbp::sonata::CircuitConfig& config,
                         const brayns::PropertyMap& props,
                         const size_t numNodes,
                         const std::string& property,
                         const std::string& debugName)
{
    // CHECK AFFERENT AND EFFERENT EDGE POPULATIONS
    const auto diskEdgePopulations = config.listEdgePopulations();
    const auto& edgePops = props.getPropertyRef<string_list>(property);
    if(edgePops.size() != numNodes)
        throw std::invalid_argument("A comma-separated list of " + debugName + " edge populations"
                                    " must be specified, one per population (or an empty string)");
    for(const auto& edgePopList : edgePops)
    {
        const auto edgePopTokens = brayns::string_utils::split(edgePopList, ',');
        const std::unordered_set<std::string> uniqueEdgePopTokens (edgePopTokens.begin(),
                                                                       edgePopTokens.end());
        for(const auto& name : uniqueEdgePopTokens)
        {
            if(diskEdgePopulations.find(name) == diskEdgePopulations.end())
                throw std::invalid_argument(debugName + " edge population '" + name
                                            + "' not found in network");
        }
    }
}

void checkEdges(const bbp::sonata::CircuitConfig& config,
                const brayns::PropertyMap& props,
                const size_t numNodes)
{
    // CHECK AFFERENT/EFFERENT
    checkEdgePopulation(config,
                        props,
                        numNodes,
                        PROPERTY_AFFERENTPOPULATIONS.name,
                        "afferent");
    checkEdgePopulation(config,
                        props,
                        numNodes,
                        PROPERTY_EFFERENTPOPULATIONS.name,
                        "efferent");

    const auto& edgePercents = props.getPropertyRef<string_list>(PROPERTY_SYNAPSEPERCENTAGE.name);
    if(edgePercents.size() != numNodes)
        throw std::invalid_argument("A list of synapse load percentage must be specified, "
                                    "one per population (must be greather than or equal to 0)");
    for(const auto& percent : edgePercents)
    {
        if(std::stof(percent) < 0.f)
            throw std::invalid_argument("Synapse load percentage must be a value between 0 and 1");
    }
}

/**
 * @brief Checks the sanity of the parameter to configure the loading of node sets
 */
void checkNodeSets(const bbp::sonata::CircuitConfig& config,
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
        if(!boost::filesystem::exists(config.getNodeSetsPath()))
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

void checkMorphologyParts(const brayns::PropertyMap& props,
                          const size_t numNodes)
{
    const auto& parts = props.getPropertyRef<string_list>(PROPERTY_MORPHOLOGYPARTS.name);
    if(parts.size() != numNodes)
        throw std::invalid_argument("A comma-separated list of morphology parts must be specified, "
                                    "one per population (or an empty string to load all)");

    for(const auto& part : parts)
    {
        string_list partTokens = brayns::string_utils::split(part, ',');
        for(const auto& partIndexStr : partTokens)
        {
            uint32_t partIndex = std::numeric_limits<uint32_t>::max();
            try
            {
                partIndex = std::stoi(partIndexStr);
            }
            catch(const std::invalid_argument& e) {}

            if(partIndex > 4)
                throw std::invalid_argument("Unknown morphology part index to load '"
                                            + partIndexStr + "'");
        }
    }

    const auto& radiusMultiplier = props.getPropertyRef<string_list>(
                                            PROPERTY_MORPHOLOGYRADIUSMULT.name);
    if(radiusMultiplier.size() != numNodes)
        throw std::invalid_argument("A list of morphology radius multiplier must be specified, "
                                    "one per population (must be greather than 0)");
    for(const auto& multiplier : radiusMultiplier)
    {
        if(std::stof(multiplier) <= 0.f)
            throw std::runtime_error("Morphology radius multiplier must be a value above 0");
    }

    const auto& loadMode = props.getPropertyRef<string_list>(PROPERTY_MORPHOLOGYLOADMODE.name);
    if(loadMode.size() != numNodes)
        throw std::invalid_argument("A list of morphology load modes must be specified, "
                                    "one per population (must be one of the possible values)");
    for(const auto& mode : loadMode)
    {
        if(mode != "vanilla" && mode != "smooth" && mode != "samples")
            throw std::invalid_argument("Unknown morphology load mode '" + mode +"' (possible "
                                        "values: vanilla, smooth, samples)");
    }
}

void checkNodeIds(const brayns::PropertyMap& props,
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

void checkSimulation(const brayns::PropertyMap& props,
                     const size_t numNodes)
{
    const auto& simTypes = props.getPropertyRef<string_list>(PROPERTY_NODESIMULATIONTYPE.name);
    if(simTypes.size() != numNodes)
        throw std::invalid_argument("A list of node simulation types must be specified "
                                    "for each population");

    std::vector<SimulationType> types;
    types.reserve(numNodes);
    for(const auto& simTypeStr : simTypes)
    {
        try
        {
            auto test = static_cast<uint8_t>(std::stoul(simTypeStr));
            if(test > 5)
                throw std::invalid_argument("Simulation type is out of the possible values: "
                                            "0 (none), 1 (spikes), 2 (soma/compartment), "
                                            "3 (sumation)), 4 (synapse), 5 (bloodflow)");
            types.push_back(static_cast<SimulationType>(test));
        }
        catch (const std::exception& e)
        {
            throw std::invalid_argument("Cannot parse simulation type '" + simTypeStr + ": " +
                                        std::string(e.what()));
        }
    }

    const auto& simPaths = props.getPropertyRef<string_list>(PROPERTY_NODESIMULATIONFILEPATH.name);
    if(simPaths.size() != numNodes)
        throw std::invalid_argument("A list of node simulation paths must be specified "
                                    "for each population (or an empty string if the repective "
                                    "simulation type is 0 = None)");
    for(size_t i = 0; i < simPaths.size(); ++i)
    {
        if(types[i] != SimulationType::NONE && (simPaths.empty() ||
                !boost::filesystem::exists(simPaths[i])))
            throw std::invalid_argument("Cannot find simulation file " + simPaths[i]);
    }
}

void checkVasculature(const brayns::PropertyMap& props,
                      const size_t numNodes)
{
    const auto& vascParts = props.getPropertyRef<string_list>(PROPERTY_VASCULATUREPARTS.name);
    if(vascParts.size() != numNodes)
        throw std::invalid_argument("A comma-separated list of vasculature parts must be specified, "
                                    "one per population (or an empty string to load all)");

    for(const auto& part : vascParts)
    {
        string_list partTokens = brayns::string_utils::split(part, ',');
        for(const auto& partIndexStr : partTokens)
        {
            uint32_t partIndex = std::numeric_limits<uint32_t>::max();
            try
            {
                partIndex = std::stoi(partIndexStr);
            }
            catch(const std::invalid_argument& e) {}

            if(partIndex > 7)
                throw std::invalid_argument("Unknown vasculature part index to load '"
                                            + partIndexStr + "'");
        }
    }
}

/**
 * @brief Check correctness of input loader parameter and disk files
 */
void checkParameters(const bbp::sonata::CircuitConfig& config,
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
    checkNodeSets(config, props, numNodePopulations);
    // CHECK EDGES
    checkEdges(config, props, numNodePopulations);
    // CHECK MORPHOLOGY PARTS
    checkMorphologyParts(props, numNodePopulations);
    // CHECK NODE IDS
    checkNodeIds(props, numNodePopulations);
    // CHECK REPORTS
    checkSimulation(props, numNodePopulations);
    // CHECK VASCULATURE
    checkVasculature(props, numNodePopulations);
}

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
uint8_t convert(const std::string& src)
{
    return static_cast<uint8_t>(std::stoul(src));
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
                dst[j] = convert<T>(tokenList[j]);
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
            result[i] = convert<T>(list[i]);
    }
    return result;
}

} // namespace

SonataLoaderProperties::SonataLoaderProperties(const bbp::sonata::CircuitConfig& config,
                                               const brayns::PropertyMap& properties)
{
    checkParameters(config, properties, getPropertyList());

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
    const auto simTypes = parseFlatStringList<uint8_t>(simTypesRaw);

    const auto simPaths =
            properties.getPropertyRef<string_list>(PROPERTY_NODESIMULATIONFILEPATH.name);

    const auto& afferentPopulationsRaw =
            properties.getPropertyRef<string_list>(PROPERTY_AFFERENTPOPULATIONS.name);
    const auto afferentPopulations = parseStringList<std::string>(afferentPopulationsRaw);

    const auto& efferentPopulationsRaw =
            properties.getPropertyRef<string_list>(PROPERTY_EFFERENTPOPULATIONS.name);
    const auto efferentPopulations = parseStringList<std::string>(efferentPopulationsRaw);

    const auto& synapsePercentsRaw =
            properties.getPropertyRef<string_list>(PROPERTY_SYNAPSEPERCENTAGE.name);
    const auto synapsePercents = parseFlatStringList<float>(synapsePercentsRaw);

    const auto& morphologySectionsRaw =
            properties.getPropertyRef<string_list>(PROPERTY_MORPHOLOGYPARTS.name);
    const auto morphologySections = parseStringList<uint8_t>(morphologySectionsRaw);

    const auto& morphologyRadiusMultRaw =
            properties.getPropertyRef<string_list>(PROPERTY_MORPHOLOGYRADIUSMULT.name);
    const auto morphologyRadiusMult = parseFlatStringList<float>(morphologyRadiusMultRaw);

    const auto& morphologyLoadMode =
            properties.getPropertyRef<string_list>(PROPERTY_MORPHOLOGYLOADMODE.name);

    const auto& vasculatureSectionsRaw =
            properties.getPropertyRef<string_list>(PROPERTY_VASCULATUREPARTS.name);
    const auto vasculatureSections = parseStringList<uint8_t>(vasculatureSectionsRaw);

    _nodePopulations.resize(populationList.size());
    for(size_t i = 0; i < populationList.size(); ++i)
    {
        auto& pls = _nodePopulations[i];
        pls.name = populationList[i];
        pls.nodeIds = nodeIds[i];
        pls.nodeSets = nodeSets[i];
        pls.percentage = glm::clamp(nodeLoadPercentages[i], 0.f, 1.f);
        pls.simulationType = static_cast<SimulationType>(simTypes[i]);
        pls.simulationPath = simPaths[i];
        pls.afferentPopulations = afferentPopulations[i];
        pls.efferentPopulations = efferentPopulations[i];
        pls.edgePercentage = glm::clamp(synapsePercents[i], 0.f, 1.f);
        pls.morphologyRadius = morphologyRadiusMult[i];
        pls.morphologyMode = morphologyLoadMode[i];

        for(const auto morphologyPart : morphologySections[i])
            pls.morphologySections.insert(static_cast<MorphologySection>(morphologyPart));

        for(const auto vasculaturePart : vasculatureSections[i])
            pls.vasculatureSections.insert(static_cast<VasculatureSection>(vasculaturePart));
    }
}

brayns::PropertyMap SonataLoaderProperties::getPropertyList() noexcept
{
    brayns::PropertyMap props;
    props.setProperty(PROPERTY_NODEPOPULATIONS);
    props.setProperty(PROPERTY_NODESETS);
    props.setProperty(PROPERTY_NODEPERCENTAGE);
    props.setProperty(PROPERTY_NODEIDS);
    props.setProperty(PROPERTY_NODESIMULATIONTYPE);
    props.setProperty(PROPERTY_NODESIMULATIONTYPE);
    props.setProperty(PROPERTY_AFFERENTPOPULATIONS);
    props.setProperty(PROPERTY_EFFERENTPOPULATIONS);
    props.setProperty(PROPERTY_SYNAPSEPERCENTAGE);
    props.setProperty(PROPERTY_MORPHOLOGYLOADMODE);
    props.setProperty(PROPERTY_MORPHOLOGYPARTS);
    props.setProperty(PROPERTY_MORPHOLOGYRADIUSMULT);
    props.setProperty(PROPERTY_VASCULATUREPARTS);
    return props;
}

const std::vector<PopulationLoadConfig>&
SonataLoaderProperties::getRequestedPopulations() const noexcept
{
    return _nodePopulations;
}
