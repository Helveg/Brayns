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

#include "SonataNGVLoader.h"

#include "../AdvancedCircuitLoader.h"

#include <common/log.h>
#include <common/types.h>

#include <brion/blueConfig.h>

SonataNGVLoader::SonataNGVLoader(brayns::Scene &scene,
                                 const brayns::ApplicationParameters &applicationParameters,
                                 brayns::PropertyMap &&loaderParams,
                                 CircuitExplorerPlugin* plugin)
    : AbstractCircuitLoader(scene, applicationParameters,
                            std::move(loaderParams), plugin)
{
    PLUGIN_INFO << "Registering " << getName() << std::endl;
    _fixedDefaults.setProperty(
        {PROP_PRESYNAPTIC_NEURON_GID.name, std::string("")});
    _fixedDefaults.setProperty(
        {PROP_POSTSYNAPTIC_NEURON_GID.name, std::string("")});
    _fixedDefaults.setProperty(PROP_SYNCHRONOUS_MODE);
}

std::string SonataNGVLoader::getName() const
{
    return std::string("Sonata NGV circuit loader");
}

brayns::PropertyMap SonataNGVLoader::getCLIProperties()
{
    brayns::PropertyMap properties;
    properties.setProperty({"populations", std::vector<std::string>(), {"Populations to load"}});
    properties.setProperty({"reports", std::vector<std::string>(), {"Reports to load"}});
    properties.setProperty({"reportTypes", std::vector<std::string>(), {"Report types to load"}});
    auto pm = AdvancedCircuitLoader::getCLIProperties();
    properties.merge(pm);
    return properties;
}

std::vector<brayns::ModelDescriptorPtr>
SonataNGVLoader::importFromFile(const std::string& file,
                                const brayns::LoaderProgress& cb,
                                const brayns::PropertyMap& props) const
{
    PLUGIN_INFO << "Loading " << file << std::endl;
    std::vector<brayns::ModelDescriptorPtr> result;

    const std::vector<std::string>& populationNames =
            props.getPropertyRef<std::vector<std::string>>("populations");
    const std::vector<std::string>& populationReports =
            props.getPropertyRef<std::vector<std::string>>("reports");
    const std::vector<std::string>& populationReportTypes =
            props.getPropertyRef<std::vector<std::string>>("reportTypes");
    const double density = props.getProperty<double>(PROP_DENSITY.name);

    if(populationNames.size() != populationReports.size()
        || populationNames.size() != populationReportTypes.size())
        PLUGIN_THROW("Population name count must match report name, report type count")

    for(size_t i = 0; i < populationNames.size(); ++i)
    {
        // Default properties used for each loaded population
        brayns::PropertyMap defaultProperties = AdvancedCircuitLoader::getCLIProperties();
        defaultProperties.updateProperty(PROP_SECTION_TYPE_APICAL_DENDRITE.name, true);
        defaultProperties.updateProperty(PROP_SECTION_TYPE_AXON.name, false);
        defaultProperties.updateProperty(PROP_SECTION_TYPE_DENDRITE.name, true);
        defaultProperties.updateProperty(PROP_SECTION_TYPE_SOMA.name, true);
        defaultProperties.updateProperty(PROP_USER_DATA_TYPE.name, std::string("Simulation offset"));
        defaultProperties.updateProperty(PROP_LOAD_LAYERS.name, false);
        defaultProperties.updateProperty(PROP_LOAD_ETYPES.name, false);
        defaultProperties.updateProperty(PROP_LOAD_MTYPES.name, false);
        defaultProperties.updateProperty(PROP_USE_SDF_GEOMETRY.name, true);
        defaultProperties.setProperty(PROP_PRESYNAPTIC_NEURON_GID);
        defaultProperties.setProperty(PROP_POSTSYNAPTIC_NEURON_GID);

        // Population variables
        const auto& populationName = populationNames[i];
        const auto& populationReport = populationReports[i];
        const auto& populationReportType = populationReportTypes[i];

        PLUGIN_INFO << "Loading population " << populationName << std::endl;

        // Use the default parameters and update the variable ones for each population
        defaultProperties.updateProperty(PROP_DENSITY.name, density);
        defaultProperties.updateProperty(PROP_REPORT.name, populationReport);
        defaultProperties.updateProperty(PROP_REPORT_TYPE.name, populationReportType);

        // Load the BlueConfig/CircuitConfig
        std::unique_ptr<brion::BlueConfig> config;
        // Section Run Default
        if(populationName == "Default")
            config = std::make_unique<brion::BlueConfig>(file);
        // Section Circuit <population name>
        else
            config = std::make_unique<brion::BlueConfig>(
                        file, brion::BlueConfigSection::CONFIGSECTION_CIRCUIT, populationName);

        // Import the model
        auto model = importCircuitFromBlueConfig(*config, defaultProperties, cb);
        if(model)
            result.push_back(model);

    }

    PLUGIN_INFO << "Done" << std::endl;

    return result;
}

