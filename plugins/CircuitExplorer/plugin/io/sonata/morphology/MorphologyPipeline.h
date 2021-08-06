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

#include "../SonataLoaderTypes.h"

#include <type_traits>
#include <unordered_map>

/**
 * @brief The MorphologyPipelineStage class is the base class to implement
 *        morphology processing stages
 */
class MorphologyPipelineStage
{
public:
    virtual ~MorphologyPipelineStage() = default;

    virtual void proccess(Morphology& morphology) const = 0;
};

/**
 * @brief The MorphologyPipeline class implements a configurable pipeline that
 *        allows to process a Morphology object before is being converted to
 *        a 3D shape
 */
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
    static void registerBuilder(const std::string& name)
    {
        static_assert(std::is_base_of<MorphologyGeometryBuilder, BuilderClass>::value,
                      "Attempted to register wrong type of geometry builder");

        _builders[name] = []() { return std::make_unique<BuilderClass>(); };
    }

    std::unique_ptr<MorphologyGeometryBuilder>
    createMorphologyBuilder(const std::string& builderName,
                            const std::string& morphologyPath,
                            const std::unordered_set<MorphologySection>& morphologyParts) const
    {
        auto it = _builders.find(builderName);
        if(it == _builders.end())
            throw std::runtime_error("MorphologyPipeline: Unknown builder type '"
                                     + builderName + "'");

        Morphology m (morphologyPath, morphologyParts);

        for(const auto& stage : _stages)
            stage->proccess(m);
        auto builder = (it->second)();
        builder->build(m);
        return builder;
    }

private:
    using BuilderPtr = std::unique_ptr<MorphologyGeometryBuilder>;
    static std::unordered_map<std::string, std::function<BuilderPtr()>> _builders;

    std::vector<std::unique_ptr<MorphologyPipelineStage>> _stages;
};
