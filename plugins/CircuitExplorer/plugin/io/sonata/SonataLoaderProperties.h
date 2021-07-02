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

#pragma once

#include <brayns/common/PropertyMap.h>

#include <bbp/sonata/config.h>

#include "../morphology/MorphologyTypes.h"
#include "data/SonataTypes.h"

#include <unordered_set>

namespace sonata
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
            {"List of simulation types to load (will be used to interpret the files specified by "
             "NodeSimulationFilePath). Possible values: 0 (do not load simulation), 1 (spikes), "
             "2 (soma/full compartment)"}
        };

const brayns::Property PROPERTY_NODESIMULATIONFILEPATH = {
            "NodeSimulationFilePath",
            std::vector<std::string>(),
            {"List of paths to compartment or spike report files to be loaded for each "
             "population (or an empty string to not load any)"}
        };

const brayns::Property PROPERTY_AFFERENTPOPULATIONS = {
            "AfferentEdgePopulations",
            std::vector<std::string>(),
            {"Comma-separated list of edge populations from which to load afferent "
             "edges for each population. The edge population must be valid for the "
             "node population that is being requested for"}
        };

const brayns::Property PROPERTY_EFFERENTPOPULATIONS = {
            "EfferentEdgePopulations",
            std::vector<std::string>(),
            {"Comma-separated list of edge populations from which to load efferent "
             "edges for each population. The edge population must be valid for the "
              "node population that is being requested for"}
        };

const brayns::Property PROPERTY_SYNAPSEPERCENTAGE = {
            "EdgesPercentage",
            std::vector<std::string>(),
            {"Percentage of synapses to load for each population"}
};

const brayns::Property PROPERTY_MORPHOLOGYPARTS = {
            "MorphologySectionTypes",
            std::vector<std::string>(),
            {"A comma separated list of values that represent sections of the morphology "
             "to load (0 = soma, 1 = axons, 2 = dendrites, 3 = apical dendrites)"}
        };

const brayns::Property PROPERTY_MORPHOLOGYRADIUSMULT = {
            "MorphologyRadiusMultiplier",
            std::vector<std::string>(),
            {"A value used to multiply all morphology sample radii by"}
        };

const brayns::Property PROPERTY_MORPHOLOGYLOADMODE = {
            "MorphologyLoadMode",
            std::vector<std::string>(),
            {"Method to load and display the morphology. Possible values are: "
             "'vanilla' (as read from disk), "
             "'smooth', (samples radii is adjusted for a smooth result) "
             "'samples' (each sample is represented with a sphere)"}
};

/**
 * @brief The PopulationLoadConfig struct holds the parsed information specified by the
 *        user to configure the loading of a node population
 */
struct PopulationLoadConfig
{
    std::string name;
    float percentage;
    std::vector<uint64_t> nodeIds;
    std::vector<std::string> nodeSets;
    sonata::data::SimulationType simulationType;
    std::string simulationPath;
    std::vector<std::string> afferentPopulations;
    std::vector<std::string> efferentPopulations;
    float edgePercentage;
    float morphologyRadius;
    std::unordered_set<morphology::SectionType> morphologySections;
    std::string morphologyMode;
};

/**
 * @brief The SonataLoaderProperties class is in charge of checking the correctness
 *        of the input parameters for the sonata loader, as well as extract the information
 *        from them and make them available in the format they are needed
 */
class SonataLoaderProperties
{
public:
    SonataLoaderProperties(const bbp::sonata::CircuitConfig& config,
                           const brayns::PropertyMap& properties);

    static brayns::PropertyMap getPropertyList() noexcept;

    const std::vector<PopulationLoadConfig>& nodePopulations() const noexcept;
private:

    std::vector<PopulationLoadConfig> _nodePopulations;
    std::unordered_set<morphology::SectionType> _morphologySections;
    double _morphologyRadius;
};
}
