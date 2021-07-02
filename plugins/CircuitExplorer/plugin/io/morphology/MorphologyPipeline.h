/* Copyright (c) 2015-2021, EPFL/Blue Brain Project
 * All rights reserved. Do not distribute without permission.
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

#include "Morphology.h"
#include "MorphologyGeometryBuilder.h"
#include "MorphologyInstance.h"

#include <type_traits>

namespace morphology
{
class MorphologyPipelineStage
{
public:
    virtual ~MorphologyPipelineStage() = default;

    virtual void proccess(Morphology& morphology) const = 0;
};

class AbstractBuilderFactory
{
public:
    virtual std::unique_ptr<MorphologyGeometryBuilder> create() const = 0;
};

template<class Builder>
class BuilderFactory : public AbstractBuilderFactory
{
public:
    std::unique_ptr<MorphologyGeometryBuilder> create() const final
    {
        return std::make_unique<Builder>();
    }
};

class MorphologyPipeline
{
public:
    template<typename StageClass, typename ...Args>
    void registerStage(Args&&...args)
    {
        static_assert (std::is_base_of<MorphologyPipelineStage, StageClass>::value,
                       "Attempted to register wrong type of morphology stage");

        _stages.push_back(std::make_unique<StageClass>(std::forward<Args>(args)...));
    }

    template<typename BuilderClass>
    void setGeometryBuilderClass()
    {
        static_assert(std::is_base_of<MorphologyGeometryBuilder, BuilderClass>::value,
                      "Attempted to register wrong type of geometry builder");

        _builderFactory = std::make_unique<BuilderFactory<BuilderClass>>();
    }

    std::unique_ptr<MorphologyGeometryBuilder>
    importMorphology(const std::string& path,
                     const std::unordered_set<SectionType>& sections) const
    {
        if(!_builderFactory)
            throw std::runtime_error("MorphologyPipeline: No geometry builder class setted!");

        Morphology m (path, sections);
        for(const auto& stage : _stages)
            stage->proccess(m);
        auto builder = _builderFactory->create();
        builder->build(m);
        return builder;
    }

private:
    std::unique_ptr<AbstractBuilderFactory> _builderFactory;
    std::vector<std::unique_ptr<MorphologyPipelineStage>> _stages;
};
}
