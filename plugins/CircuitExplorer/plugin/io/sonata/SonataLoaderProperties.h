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

#include <plugin/io/sonata/SonataLoaderEnums.h>

#include <brayns/common/PropertyMap.h>

#include <bbp/sonata/config.h>

#include <unordered_set>

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
            {"List of simulation types numeric ID to load (will be used to interpret the files "
             "specified by NodeSimulationFilePath). Possible values: 0 (none), 1 (spike), "
             "2 (compartment), 3 (summation), 4 (synapse) or 5 (bloodflow)"}
        };

const brayns::Property PROPERTY_NODESIMULATIONFILEPATH = {
            "NodeSimulationFilePath",
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

const brayns::Property PROPERTY_NEURONPARTS = {
            "NeuronSectionTypes",
            std::vector<std::string>(),
            {"A comma separated list of numeric values that represent sections of the neuron and "
             "astroctytes to load (0 = soma, 1 = axon, 2 = basal dendrite, 3 = apical dendrite)"}
        };

const brayns::Property PROPERTY_NEURONLOADMODE = {
            "NeuronLoadMode",
            std::vector<std::string>(),
            {"Method to load and display the neurons and astrocytes. Possible values are: "
             "'vanilla' (as read from disk), "
             "'smooth', (samples radii is adjusted for a smooth result) "
             "'samples' (each sample is represented with a sphere)"}
        };

const brayns::Property PROPERTY_VASCULATUREPARTS = {
            "VasculatureTypes",
            std::vector<std::string>(),
            {"A comma separated list of numeric values that represent sections of the vasculature "
             "to load (0 = all, 1 = vein, 2 = artery, 3 = venule, 4 = arteroile, "
             "5 = venous-capillary, 6 = arterial-capillary, 7 = transitional)"}
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
    std::set<NeuronSection> sections;
    std::string mode;
};

/**
 * @brief The VasculatureLoadConfig struct configures how to load vasculature morphologies
 */
struct VasculatureLoadConfig
{
    float radiusMultiplier;
    std::set<VasculatureSection> sections;
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
    SonataLoaderProperties(const std::string& path,
                           const bbp::sonata::CircuitConfig& config,
                           const brayns::PropertyMap& properties);

    static brayns::PropertyMap getPropertyList() noexcept;

    const std::vector<PopulationLoadConfig>& getRequestedPopulations() const noexcept;
private:
    std::vector<PopulationLoadConfig> _nodePopulations;
};
