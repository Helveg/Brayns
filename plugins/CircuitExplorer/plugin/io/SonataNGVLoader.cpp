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

#include "SonataNGVLoader.h"

#include <brayns/common/Timer.h>
#include <brayns/common/utils/stringUtils.h>

#include <plugin/api/Log.h>
#include <plugin/io/BBPLoader.h>
#include <plugin/io/bbploader/BBPLoaderProperties.h>
#include <plugin/io/bbploader/simulation/SimulationType.h>

#include <brion/blueConfig.h>

namespace
{
constexpr char PROP_POPULATIONS_NAME[] = "Populations";
constexpr char PROP_REPORTS_NAME[]     = "Reports";
constexpr char PROP_REPORT_TYPES_NAME[]= "ReportTypes";

inline void __checkInput(const brayns::PropertyMap& input)
{
    const std::vector<std::string> props = {PROP_POPULATIONS_NAME,
                                            PROP_REPORTS_NAME,
                                            PROP_REPORT_TYPES_NAME};
    for(size_t i = 0; i < 3; ++i)
    {
        if(!input.hasProperty(props[i]))
            throw std::invalid_argument("SonataNGVLoader: missing property " + props[i]);
    }
}
}

SonataNGVLoader::SonataNGVLoader(brayns::Scene& scene, CircuitColorManager& colorManager)
 : brayns::Loader(scene)
 , _internal(scene, colorManager)
{
    PLUGIN_INFO << "Registering loader: " << getName() << std::endl;
}

std::vector<std::string> SonataNGVLoader::getSupportedExtensions() const
{
    return _internal.getSupportedExtensions();
}

bool SonataNGVLoader::isSupported(const std::string& filename, const std::string& extension) const
{
    return _internal.isSupported(filename, extension);
}

std::string SonataNGVLoader::getName() const
{
    return std::string("SONATA NGV loader");
}

brayns::PropertyMap SonataNGVLoader::getProperties() const
{
    // Get all properties except report configuration
    auto bbploaderProps = bbploader::BBPLoaderProperties::getPropertyList();
    brayns::PropertyMap defaultProps;
    for(const auto& property : bbploaderProps.getProperties())
    {
        if(property->name == bbploader::PROP_REPORT.name
                || property->name == bbploader::PROP_REPORT_TYPE.name)
            continue;
        defaultProps.setProperty(*property);
    }

    defaultProps.setProperty({PROP_POPULATIONS_NAME,
                              std::vector<std::string>(),
                              {"List of populations to load"}});
    defaultProps.setProperty({PROP_REPORTS_NAME,
                             std::vector<std::string>(),
                              {"List of report names to load, one per population"}});
    defaultProps.setProperty({PROP_REPORT_TYPES_NAME,
                             std::vector<std::string>(),
                              {"List of report types for each specified report name. Possible "
                               "values are: " + brayns::string_utils::join(
                               EnumWrapper<bbploader::SimulationType>().toStringList(), ",")}});
    return defaultProps;
}

std::vector<brayns::ModelDescriptorPtr>
SonataNGVLoader::importFromBlob(brayns::Blob&&,
                                const brayns::LoaderProgress&,
                                const brayns::PropertyMap&) const
{
    throw std::runtime_error("SonataNGVLoader: Import from blob not supported");
}

std::vector<brayns::ModelDescriptorPtr>
SonataNGVLoader::importFromFile(const std::string& path,
                                const brayns::LoaderProgress& cb,
                                const brayns::PropertyMap& props) const
{
    brayns::Timer timer;
    PLUGIN_INFO << getName() << ": Loading " << path << std::endl;

     __checkInput(props);

    std::vector<brayns::ModelDescriptorPtr> result;

    const std::vector<std::string>& populationNames =
            props.getPropertyRef<std::vector<std::string>>(PROP_POPULATIONS_NAME);
    const std::vector<std::string>& reportNames =
            props.getPropertyRef<std::vector<std::string>>(PROP_REPORTS_NAME);
    const std::vector<std::string>& reportTypes =
            props.getPropertyRef<std::vector<std::string>>(PROP_REPORT_TYPES_NAME);


    if(populationNames.size() != reportNames.size()
        || populationNames.size() != reportTypes.size())
        PLUGIN_THROW("'Populations' count must match 'Reports' and 'ReportTypes' count");

    for(size_t i = 0; i < populationNames.size(); ++i)
    {
        // Default properties used for each loaded population
        brayns::PropertyMap defaultProperties = props;
        defaultProperties.setProperty({bbploader::PROP_REPORT.name, reportNames[i]});
        defaultProperties.setProperty({bbploader::PROP_REPORT_TYPE.name, reportTypes[i]});

        // Population variables
        const auto& populationName = populationNames[i];

        PLUGIN_INFO << "\tSonata NGV Loader: Loading population " << populationName << std::endl;

        // Load the BlueConfig/CircuitConfig
        std::unique_ptr<brion::BlueConfig> config;
        if(populationName == "Default")
            config = std::make_unique<brion::BlueConfig>(path);
        else
            config = std::make_unique<brion::BlueConfig>(
                        path, brion::BlueConfigSection::CONFIGSECTION_CIRCUIT, populationName);

        // Import the model
        auto models = _internal.importFromBlueConfig(path, cb, defaultProperties, *config);
        for(auto& model : models)
            model->setName(populationName + " - " + model->getName());
        result.insert(result.end(), models.begin(), models.end());
    }

    PLUGIN_INFO << getName() << ": Done in " << timer.elapsed() << " second(s)" << std::endl;
    return result;
}
