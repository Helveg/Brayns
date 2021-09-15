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

#include "NeuronMorphologyLoader.h"

#include <brayns/common/Timer.h>
#include <brayns/common/utils/stringUtils.h>
#include <brayns/engine/Model.h>
#include <brayns/engine/Scene.h>

#include <plugin/api/Log.h>
#include <plugin/io/morphology/neuron/NeuronBuilder.h>
#include <plugin/io/morphology/neuron/NeuronMorphology.h>
#include <plugin/io/morphology/neuron/NeuronMorphologyPipeline.h>
#include <plugin/io/morphology/neuron/builders/PrimitiveNeuronBuilder.h>
#include <plugin/io/morphology/neuron/builders/SDFNeuronBuilder.h>
#include <plugin/io/morphology/neuron/builders/SampleNeuronBuilder.h>
#include <plugin/io/morphology/neuron/pipeline/RadiusMultiplier.h>
#include <plugin/io/morphology/neuron/pipeline/RadiusOverride.h>
#include <plugin/io/morphology/neuron/pipeline/RadiusSmoother.h>
#include <plugin/io/morphology/neuron/NeuronGeometryType.h>
#include <plugin/io/morphology/neuron/NeuronSection.h>

namespace
{
constexpr char PROP_GEOMETRY_MODE_NAME[]    = "GeometryMode";
constexpr char PROP_SECTIONS_NAMES[]        = "NeuronSections";
constexpr char PROP_RADIUSMULTIPLIER_NAME[] = "RadiusMultiplier";
constexpr char PROP_RADIUSOVERRIDE_NAME[]   = "RadiusOverride";

inline void __checkInput(const brayns::PropertyMap& defProps, const brayns::PropertyMap& src)
{
    for(const auto& prop : defProps.getProperties())
    {
        if(!src.hasProperty(prop->name))
            throw std::invalid_argument("NeuronMorphologyLoader: Missing property " + prop->name);
    }
}

inline NeuronGeometryType __parseGeometryMode(const brayns::PropertyMap& input)
{
    const auto& mode = input.getPropertyRef<std::string>(PROP_GEOMETRY_MODE_NAME);
    if(mode.empty())
        throw std::invalid_argument("NeuronMorphologyLoader: A geometry mode must be specified");
    return EnumWrapper<NeuronGeometryType>().fromString(mode);
}

inline std::unique_ptr<NeuronBuilder> __instantiateBuilder(const NeuronGeometryType geomType,
                                                           const NeuronSection sections) noexcept
{
    if(sections == NeuronSection::SOMA)
        return std::make_unique<SampleNeuronBuilder>();

    switch(geomType)
    {
    case NeuronGeometryType::SAMPLES:
        return std::make_unique<SampleNeuronBuilder>();
    case NeuronGeometryType::SMOOTH:
        return std::make_unique<SDFNeuronBuilder>();
    default:
        return std::make_unique<PrimitiveNeuronBuilder>();
    }
}

inline NeuronSection __parseNeuronSections(const brayns::PropertyMap& input)
{
    const auto& rawParts = input.getPropertyRef<std::string>(PROP_SECTIONS_NAMES);
    if(rawParts.empty())
        throw std::invalid_argument("NeuronMorphologyLoader: At least one section to load "
                                    "must be specified");

    const auto parts = brayns::string_utils::split(rawParts, ',');
    const EnumWrapper<NeuronSection> sectionEnum;
    NeuronSection result = NeuronSection::NONE;
    for(const auto& part : parts)
        result |= sectionEnum.fromString(part);

    if(result == NeuronSection::NONE)
        throw std::invalid_argument("NeuronMorphologyLoader: At least one section to load "
                                    "must be specified");

    return result;
}

inline NeuronMorphologyPipeline __instantiatePipeline(const NeuronGeometryType type,
                                                      const NeuronSection sections,
                                                      const float radiusMultiplier,
                                                      const float radiusOverride) noexcept
{
    NeuronMorphologyPipeline pipeline;
    if(radiusOverride > 0.f)
        pipeline.registerStage<RadiusOverride>(radiusOverride);
    else
    {
        if(radiusMultiplier != 1.f)
            pipeline.registerStage<RadiusMultiplier>(radiusMultiplier);
        if(type == NeuronGeometryType::SMOOTH && sections != NeuronSection::SOMA)
            pipeline.registerStage<RadiusSmoother>();
    }
    return pipeline;
}

inline float __parseRadiusMultiplier(const brayns::PropertyMap& input)
{
    const auto v = static_cast<float>(input.getProperty<double>(PROP_RADIUSMULTIPLIER_NAME, 1.0));
    if(v <= 0.f)
        throw std::invalid_argument("NeuronMorphologyLoader: Radius multiplier must be above 0");
    return v;
}

inline float __parseRadiusOverride(const brayns::PropertyMap& input)
{
    const auto v = static_cast<float>(input.getProperty<double>(PROP_RADIUSOVERRIDE_NAME, 0.0));
    if(v < 0.f)
        throw std::invalid_argument("NeuronMorphologyLoader: Radius override must be >= 0");
    return v;
}
}

NeuronMorphologyLoader::NeuronMorphologyLoader(brayns::Scene& scene)
 : brayns::Loader(scene)
{
    PLUGIN_INFO << "Registering loader: " << getName() << std::endl;
}

std::vector<std::string> NeuronMorphologyLoader::getSupportedExtensions() const
{
    return {".swc", ".h5", ".asc"};
}

bool NeuronMorphologyLoader::isSupported(const std::string& filename,
                                         const std::string& extension) const
{
    for(const auto& ext : getSupportedExtensions())
    {
        if(ext.find(extension) != std::string::npos
                || filename.find(ext))
            return true;
    }

    return false;
}

std::string NeuronMorphologyLoader::getName() const
{
    return "Neuron Morphology loader";
}

brayns::PropertyMap NeuronMorphologyLoader::getProperties() const
{
    brayns::PropertyMap properties;
    properties.setProperty({PROP_GEOMETRY_MODE_NAME,
                            std::string(),
                            {"Method to load and display the neurons and astrocytes. Possible "
                             "values are: " + brayns::string_utils::join(
                             EnumWrapper<NeuronGeometryType>().toStringList(), ",")}});
    properties.setProperty({PROP_SECTIONS_NAMES,
                            std::string(),
                            {"Comma separated list of sections to load. Possible values are "
                             + brayns::string_utils::join(
                             EnumWrapper<NeuronSection>().toStringList(), ",")}});
    properties.setProperty({PROP_RADIUSMULTIPLIER_NAME,
                            1.0,
                            {"A value used to multiply all geometry sample radii by"}});
    return properties;

}

std::vector<brayns::ModelDescriptorPtr>
NeuronMorphologyLoader::importFromBlob(brayns::Blob&&,
                                       const brayns::LoaderProgress&,
                                       const brayns::PropertyMap&) const
{
    throw std::runtime_error("MorphologyLoader: Import from blob not supported");
}


std::vector<brayns::ModelDescriptorPtr>
NeuronMorphologyLoader::importFromFile(const std::string& path,
                                       const brayns::LoaderProgress& callback,
                                       const brayns::PropertyMap& properties) const
{
    brayns::Timer timer;
    PLUGIN_INFO << getName() << ": Loading " << path << std::endl;

    __checkInput(getProperties(), properties);

    callback.updateProgress("Loading " + path, 0.f);

    const auto geometryMode = __parseGeometryMode(properties);
    const auto sections = __parseNeuronSections(properties);
    const auto radMult = __parseRadiusMultiplier(properties);
    const auto radOverride = __parseRadiusOverride(properties);

    NeuronMorphology morphology (path, sections);
    const auto pipeline = __instantiatePipeline(geometryMode, sections, radMult, radOverride);
    pipeline.process(morphology);

    const auto builder = __instantiateBuilder(geometryMode, sections);
    builder->build(morphology);
    const auto morphologyGeometry = builder->instantiate(brayns::Vector3f(), brayns::Quaternion());

    auto modelPtr = _scene.createModel();
    morphologyGeometry->addToModel(*modelPtr);
    modelPtr->updateBounds();

    brayns::ModelMetadata metadata =
    {
        {"Morphology path", path},
        {"Loaded sections", properties.getPropertyRef<std::string>(PROP_SECTIONS_NAMES)},
        {"Number of sections", std::to_string(morphology.sections().size()
                                              + (morphology.hasSoma()? 1 : 0))},
    };

    brayns::Transformation transformation;
    transformation.setRotationCenter(modelPtr->getBounds().getCenter());
    auto modelDescriptor =
        std::make_shared<brayns::ModelDescriptor>(std::move(modelPtr),
                                                  "Morphology",
                                                  path,
                                                  metadata);
    modelDescriptor->setTransformation(transformation);

    PLUGIN_INFO << getName() << ": Done in " << timer.elapsed() << " second(s)" << std::endl;
    return {modelDescriptor};
}
