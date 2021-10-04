#include "SynapseLoader.h"

#include <plugin/io/synapse/groups/OldSurfaceSynapseGroup.h>

namespace bbploader
{
namespace
{
inline auto __load(const brain::Synapses& src,
                   const brain::GIDSet& gids,
                   glm::vec3 (brain::Synapse::*posMethod)() const,
                   uint32_t (brain::Synapse::*gidMethod)() const,
                   uint32_t (brain::Synapse::*sectionMethod)() const,
                   SubProgressReport& spr)
{
    std::vector<std::unique_ptr<SynapseGroup>> result;
    if(!src.empty())
    {
        std::map<uint32_t, std::unique_ptr<SynapseGroup>> synapseMap;
        for(const auto gid : gids)
            synapseMap[gid] = std::make_unique<OldSurfaceSynapseGroup>();

        for(const auto& synapse : src)
        {
            auto& group = static_cast<OldSurfaceSynapseGroup&>(
                        *synapseMap[(synapse.*gidMethod)()]);
            group.addSynapse(0, (synapse.*sectionMethod)(), (synapse.*posMethod)());
        }

        result.reserve(synapseMap.size());
        for(auto& entry : synapseMap)
        {
            result.push_back(std::move(entry.second));
            spr.tick();
        }
    }

    return result;
}
}

std::vector<std::unique_ptr<SynapseGroup>> SynapseLoader::load(const brain::Circuit& circuit,
                                                               const brain::GIDSet& gids,
                                                               const bool afferent,
                                                               SubProgressReport& spr)
{
    if(afferent)
    {
        return __load(circuit.getAfferentSynapses(gids),
                      gids,
                      &brain::Synapse::getPostsynapticSurfacePosition,
                      &brain::Synapse::getPostsynapticGID,
                      &brain::Synapse::getPostsynapticSectionID,
                      spr);
    }
    else
    {
        return __load(circuit.getEfferentSynapses(gids),
                      gids,
                      &brain::Synapse::getPresynapticSurfacePosition,
                      &brain::Synapse::getPresynapticGID,
                      &brain::Synapse::getPresynapticSectionID,
                      spr);
    }
}
}