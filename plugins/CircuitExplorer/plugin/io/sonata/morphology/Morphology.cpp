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

#include "Morphology.h"

#include <brayns/common/utils/filesystem.h>

#include <morphio/errorMessages.h>
#include <morphio/morphology.h>
#include <morphio/section.h>
#include <morphio/soma.h>

#include <mutex>


namespace
{
morphio::Morphology readMorphology(const std::string& path)
{
    static std::mutex hdf5Mutex;
    const auto ext = fs::path(path).extension().string();
    if(ext == ".h5")
    {
        std::lock_guard<std::mutex> lock(hdf5Mutex);
        return morphio::Morphology(path);
    }
    return morphio::Morphology(path);
}

std::unique_ptr<Soma> readSoma(const morphio::Morphology& m)
{
    const auto somaData = m.soma();
    const auto somaPoints = somaData.points();

    if(somaPoints.empty())
        return {nullptr};

    // Compute average position
    std::vector<brayns::Vector3f> somaSamples;
    somaSamples.reserve(somaPoints.size());
    brayns::Vector3f somaPos (0.f, 0.f, 0.f);
    for(size_t i = 0; i < somaPoints.size(); ++i)
    {
        const auto p = somaPoints[i];
        somaSamples.emplace_back(p[0], p[1], p[2]);
        somaPos.x += p[0];
        somaPos.y += p[1];
        somaPos.z += p[2];
    }
    somaPos /= static_cast<float>(somaPoints.size());

    // Compute mean radius
    float somaRadius = 0.f;
    for(const auto& somaPoint : somaSamples)
        somaRadius += glm::length(somaPoint - somaPos);
    somaRadius /= static_cast<float>(somaPoints.size());

    return std::make_unique<Soma>(somaPos, somaRadius);
}

std::vector<Section> readNeurites(const morphio::Morphology& m,
                                  const bool axon,
                                  const bool dend,
                                  const bool adend)
{
    const auto& morphSections = m.sections();

    std::vector<Section> result;
    result.reserve(morphSections.size());

    for(const auto& section : morphSections)
    {
        const auto secPoints = section.points();
        if(secPoints.empty())
            continue;

        const auto secDiameters = section.diameters();
        const auto sectionId = section.id();
        const auto parentId = section.isRoot()? -1 : section.parent().id();

        // Fill in or filter section types
        MorphologySection type;
        switch(section.type())
        {
        case morphio::SectionType::SECTION_AXON:
            if(!axon)
                continue;
            type = MorphologySection::AXON;
            break;
        case morphio::SectionType::SECTION_DENDRITE:
            if(!dend)
                continue;
            type = MorphologySection::DENDRITE;
            break;
        case morphio::SectionType::SECTION_APICAL_DENDRITE:
            if(!adend)
                continue;
            type = MorphologySection::APICAL_DENDRITE;
            break;
        default:
            break;
        }

        result.emplace_back(sectionId, parentId, type);
        Section& resultSection = result.back();

        // Fill in points
        for(size_t i = 0; i < secPoints.size(); ++i)
        {
            const auto& p = secPoints[i];
            resultSection.samples.emplace_back(p[0], p[1], p[2], secDiameters[i]);
        }
    }

    return result;
}
}

// ------------------------------------------------------------------------------------------------

Morphology::Morphology(const std::string& path,
                       const std::unordered_set<MorphologySection>& sections)
 : _morphologyPath(path)
 , _soma(nullptr)
{
    bool loadSoma, loadAxon, loadDend, loadADen;
    if(sections.find(MorphologySection::ALL) != sections.end())
        loadSoma = loadADen = loadAxon = loadDend = true;
    else
    {
        loadSoma = sections.find(MorphologySection::SOMA) != sections.end();
        loadAxon = sections.find(MorphologySection::AXON) != sections.end();
        loadDend = sections.find(MorphologySection::DENDRITE) != sections.end();
        loadADen = sections.find(MorphologySection::APICAL_DENDRITE) != sections.end();
    }

    const auto morph = readMorphology(path);

    if(loadAxon || loadADen || loadDend)
        _sections = readNeurites(morph, loadAxon, loadDend, loadADen);

    if(loadSoma)
    {
        _soma = readSoma(morph);
        if(_soma)
        {
            for(auto& section : _sections)
            {
                if(section.parentId == -1)
                    _soma->children.push_back(&section);
            }
        }
    }
}

bool Morphology::hasSoma() const noexcept
{
    return (_soma.get() != nullptr);
}

Soma& Morphology::soma()
{
    if(hasSoma())
        return *_soma;

    throw std::runtime_error("Morphology " + _morphologyPath + " loaded without soma");
}

const Soma& Morphology::soma() const
{
    if(hasSoma())
        return *_soma;

    throw std::runtime_error("Morphology " + _morphologyPath + " loaded without soma");
}

std::vector<Section>& Morphology::sections() noexcept
{
    return _sections;
}

const std::vector<Section>& Morphology::sections() const noexcept
{
    return _sections;
}

std::vector<Section*> Morphology::sectionChildren(const Section& parent) noexcept
{
    std::vector<Section*> result;
    for(auto& section : _sections)
    {
        if(section.parentId == parent.id)
            result.push_back(&section);
    }
    return result;
}

std::vector<const Section*> Morphology::sectionChildren(const Section& parent) const noexcept
{
    std::vector<const Section*> result;
    for(const auto& section : _sections)
    {
        if(section.parentId == parent.id)
            result.push_back(&section);
    }
    return result;
}

Section* Morphology::parent(const Section& section) noexcept
{
    for(auto& parent : _sections)
    {
        if(parent.id == section.parentId)
            return &parent;
    }

    return nullptr;
}

const Section* Morphology::parent(const Section& section) const noexcept
{
    for(auto& parent : _sections)
    {
        if(parent.id == section.parentId)
            return &parent;
    }

    return nullptr;
}

// ------------------------------------------------------------------------------------------------

Section::Section(const int32_t id,
                 const int32_t parentId,
                 const MorphologySection type)
 : id(id)
 , parentId(parentId)
 , type(type)
{
}

// ------------------------------------------------------------------------------------------------

Soma::Soma(const brayns::Vector3f& center,
           const float radius)
 : center(center)
 , radius(radius)
{
}
