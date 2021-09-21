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

#include <brion/blueConfig.h>

#include <brayns/common/PropertyMap.h>

#include <plugin/io/bbploader/simulation/SimulationType.h>
#include <plugin/io/morphology/neuron/NeuronGeometryType.h>
#include <plugin/io/morphology/neuron/NeuronSection.h>

namespace bbploader
{
const brayns::Property PROP_PERCENTAGE = {
    "Percentage",
    1.0,
    {"Percentage of cells of the circuit to load (0.0 - 1.0). Will not have effect if a list of "
     "GIDs is specified via 'Gids'."}};

const brayns::Property PROP_TARGETS = {
    "Targets",
    std::string(""),
    {"Circuit targets to load [comma separated list of targets]"}};

const brayns::Property PROP_GIDS = {
    "Gids",
    std::string(""),
    {"Circuit GIDs [comma separated list of GIDs]. Invalidates the 'Percentage' and 'Targets' "
     "parameters when used."}};

const brayns::Property PROP_REPORT{
    "Report",
    std::string(),
    {"Circuit soma/compartment report to load. Can be empty if 'ReportType' is 'none'"}};

const brayns::Property PROP_REPORT_TYPE = {
    "ReportType",
    std::string("None"),
    {"Type of simulation report to load. Possible values: " +
     brayns::string_utils::join(EnumWrapper<SimulationType>().toStringList(), ",")}};

const brayns::Property PROP_SPIKE_TRANSITION_TIME = {
    "SpikeTransitionTime",
    1.0,
    {"When 'ReportType' is 'spikes', controls the growth and fade of spike in seconds"}};

const brayns::Property PROP_GEOMETRY_MODE = {
    "GeometryMode",
    std::string(),
    {"Method to load and display the neuron geometry. Possible values are: "
     + brayns::string_utils::join(EnumWrapper<NeuronGeometryType>().toStringList(), ",")}};

const brayns::Property PROP_RADIUS_MULTIPLIER = {
    "RadiusMultiplier",
    1.0,
    {"Multiplier applied to morphology sample radii"}};

const brayns::Property PROP_RADIUS_OVERRIDE = {
    "RadiusOverride",
    double(0.0),
    {"Value to override the radii of the morphology samples"}};

const brayns::Property PROP_LOAD_SOMA = {
    "LoadSoma",
    true,
    {"Wether to load or not the soma section of the morphology"}};

const brayns::Property PROP_LOAD_AXON = {
    "LoadAxon",
    false,
    {"Wether to load or not the axon sections of the morphology"}};

const brayns::Property PROP_LOAD_DENDRITE = {
    "LoadDendrite",
    true,
    {"Wether to load or not the basal dendrite sections of the morphology"}};

const brayns::Property PROP_LOAD_APICAL_DENDRITE = {
    "LoadApicalDendrite",
    true,
    {"Wether to load or not the apical dendrite sections of the morphology"}};

const brayns::Property PROP_LOAD_AFFERENT_SYNAPSES = {
    "LoadAfferentSynapses",
    false,
    {"Loads afferent synapses"}};

const brayns::Property PROP_LOAD_EFFERENT_SYNAPSES = {
    "LoadEfferentSynapses",
    false,
    {"Loads efferent synapses"}};

/**
 * @brief The BBPCircuitLoadConfig struct holds all the information to load a BBP's internal
 *        format circuit
 */
struct BBPCircuitLoadConfig
{
    float percentage;
    std::vector<std::string> targets;
    std::vector<uint64_t> gids;
    std::string reportName;
    SimulationType reportType;
    float spikeTransitionTime;
    NeuronGeometryType geometryMode;
    float radiusMultiplier;
    float radiusOverride;
    NeuronSection morphologySections;
    bool loadAfferent;
    bool loadEfferent;
};

/**
 * @brief The BBPLoaderProperties class manages and gives access to the BBPLoader input properties,
 *        and check the correctness of the input parameters and files on disk before starting the
 *        load process
 */
class BBPLoaderProperties
{
public:
    /**
     * @brief return the list of all available properties for BBPLoader
     */
    static brayns::PropertyMap getPropertyList() noexcept;

    /**
     * @brief checks the sanity and parses the input parameters to load a specific circuit.
     * @throws std::invalid_argument if any sanity/parsing check fails
     */
    static BBPCircuitLoadConfig checkAndParse(const brion::BlueConfig& config,
                                              const brayns::PropertyMap& input);
};
}
