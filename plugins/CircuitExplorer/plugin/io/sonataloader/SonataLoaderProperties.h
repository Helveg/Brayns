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

#pragma once

#include <plugin/io/morphology/neuron/NeuronGeometryType.h>
#include <plugin/io/morphology/neuron/NeuronSection.h>
#include <plugin/io/morphology/vasculature/VasculatureSection.h>
#include <plugin/io/sonataloader/simulations/SimulationType.h>

#include <brayns/common/PropertyMap.h>

#include <bbp/sonata/config.h>

namespace sonataloader
{
const brayns::Property PROPERTY_NODEPOPULATIONS = {
            "NodePopulations",
            std::vector<std::string>(),
            {"List of node populations to load"}
};

const brayns::Property PROPERTY_NODESETS = {
            "NodeSets",
            std::vector<std::string>(),
            {"Comma-separated list of node sets for each population"}
};

const brayns::Property PROPERTY_NODEPERCENTAGE = {
            "NodePercentage",
            std::vector<std::string>(),
            {"Percentage (0.0 - 1.0) of all the nodes on each population to load"}
};

const brayns::Property PROPERTY_NODEIDS = {
            "NodeIds",
            std::vector<std::string>(),
            {"Comma-separated list of node ids for each population"}
};

const brayns::Property PROPERTY_NODESIMULATIONTYPE = {
            "NodeSimulationType",
            std::vector<std::string>(),
            {"Type of simulation that is contained in the specified 'NodeSimulationFilePath. "
             "Possible values are: "
             + brayns::string_utils::join(EnumWrapper<SimulationType>().toStringList(), ",")}
};

const brayns::Property PROPERTY_NODESIMULATIONFILEPATH = {
            "NodeSimulationFilepath",
            std::vector<std::string>(),
            {"List of paths to compartment or spike report files to be loaded for each "
             "population (or an empty string to not load any)"}
};

const brayns::Property PROPERTY_EDGEPOPULATIONS = {
            "EdgePopulations",
            std::vector<std::string>(),
            {"Comma-separated list of edge populations from which to load EDGES "
             "for each node population. The edge population must be valid for the "
             "node population that is being requested for (Use an empty string to "
             "avoid loading any edge population for a given node population)"}
};

const brayns::Property PROPERTY_EDGELOADMODES = {
            "EdgeLoadModes",
            std::vector<std::string>(),
            {"Comma-separated list of load modes for each edge population specified"
             "in EdgePopulations, and one entry per each node population. Available"
             "load modes are 'afferent' or 'efferent'"}
};

const brayns::Property PROPERTY_EDGEPERCENTAGES = {
            "EdgePercentages",
            std::vector<std::string>(),
            {"Comma-separated list of load percentages for each edge population specified,"
             "and one entry per each node population. Values must be in the range 0.0 - 1.0"}
};

const brayns::Property PROPERTY_EDGESIMULATIONPATHS = {
            "EdgeSimulationPaths",
            std::vector<std::string>(),
            {"Comma-separated list of paths to synapse reports to load along each specified "
             "edge population, and one entry per each nod population (Use an empty string to "
             "avoid load a report)"}
};

const brayns::Property PROPERTY_RADIUSMULT = {
            "RadiusMultiplier",
            std::vector<std::string>(),
            {"A value used to multiply all geometry sample radii by"}
};

const brayns::Property PROPERTY_RADIUSOVERRIDE = {
            "RadiusOverride",
            std::vector<std::string>(),
            {"A value used to set all geometry sample radii"}
};

const brayns::Property PROPERTY_NEURONPARTS = {
            "NeuronSections",
            std::vector<std::string>(),
            {"A comma-separated list of neuron parts to load. Possible values are: "
             + brayns::string_utils::join(EnumWrapper<NeuronSection>().toStringList(), ",")}
};

const brayns::Property PROPERTY_NEURONLOADMODE = {
            "NeuronLoadModes",
            std::vector<std::string>(),
            {"Method to load and display the neurons and astrocytes. Possible values are: "
             + brayns::string_utils::join(EnumWrapper<NeuronGeometryType>().toStringList(), ",")}
};

const brayns::Property PROPERTY_VASCULATUREPARTS = {
            "VasculatureSections",
            std::vector<std::string>(),
            {"A comma-separated list of vasculature parts to load. Possible values are: "
             + brayns::string_utils::join(EnumWrapper<VasculatureSection>().toStringList(), ",")}
};

const brayns::Property PROPERTY_VASCULATURERADIIREPORT = {
            "VasculatureRadiiReports",
            std::vector<std::string>(),
            {"List of paths to vasculature radii report files to be loaded for each "
             "population (or an empty string to not load any)"}
};


/**
 * @brief The NodeLoadConfig struct configures the node population to be loaded
 */
struct NodeLoadConfig
{
    std::string name;
    float percentage;
    std::vector<uint64_t> ids;
    std::vector<std::string> nodeSets;
    SimulationType simulationType;
    std::string simulationPath;
};

/**
 * @brief The EdgeLoadConfig struct configures each edge population to be loaded
 */
struct EdgeLoadConfig
{
    std::string name;
    bool afferent;
    float percentage;
    std::string report;
};

/**
 * @brief The NeuronLoadConfig struct configures how to load neuron morphologies
 *        (neurons / astrocytes)
 */
struct NeuronLoadConfig
{
    float radiusMultiplier;
    float radiusOverride;
    NeuronSection sections;
    NeuronGeometryType mode;
};

/**
 * @brief The VasculatureLoadConfig struct configures how to load vasculature morphologies
 */
struct VasculatureLoadConfig
{
    float radiusMultiplier;
    float radiusOverride;
    VasculatureSection sections;
    std::string radiiReport;
};

/**
 * @brief The PopulationLoadConfig struct holds the parsed information specified by the
 *        user to configure the loading of a node population
 */
struct PopulationLoadConfig
{
    std::string configPath;
    NodeLoadConfig node;
    std::vector<EdgeLoadConfig> edges;
    NeuronLoadConfig neurons;
    VasculatureLoadConfig vasculature;
};

/**
 * @brief The SonataLoaderProperties class is in charge of checking the correctness
 *        of the input parameters for the sonata loader, as well as extract the information
 *        from them and make them available in the format they are needed
 */
class SonataLoaderProperties
{
public:
    static brayns::PropertyMap getPropertyList() noexcept;

    static std::vector<PopulationLoadConfig>
    checkAndParse(const std::string& path,
                  const bbp::sonata::CircuitConfig& config,
                  const brayns::PropertyMap& properties);
};
}
