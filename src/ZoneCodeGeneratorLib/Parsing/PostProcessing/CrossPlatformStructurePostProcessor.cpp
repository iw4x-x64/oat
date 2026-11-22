#include "CrossPlatformStructurePostProcessor.h"

#include "Domain/Definition/PointerDeclarationModifier.h"

#include <unordered_set>

namespace
{
    void MarkLayoutStructureAndUsages(std::unordered_set<const void*>& visitedStructures, StructureInformation* info)
    {
        if (!visitedStructures.emplace(info).second)
            return;

        info->m_has_matching_cross_platform_structure = false;
        for (auto* usage : info->m_usages)
            MarkLayoutStructureAndUsages(visitedStructures, usage);
    }

    bool CalculateHasMatchingCrossPlatformStructure(std::unordered_set<const void*>& visitedStructures, StructureInformation* info)
    {
        if (visitedStructures.find(info) != visitedStructures.end())
            return info->m_has_matching_cross_platform_structure;

        visitedStructures.emplace(info);

        if (info->m_layout_action)
        {
            info->m_has_matching_cross_platform_structure = false;
            return false;
        }

        for (const auto& member : info->m_ordered_members)
        {
            for (const auto& modifier : member->m_member->m_type_declaration->m_declaration_modifiers)
            {
                if (modifier->GetType() == DeclarationModifierType::POINTER)
                {
                    info->m_has_matching_cross_platform_structure = false;
                    return false;
                }
            }

            if (member->m_type != nullptr && member->m_type != info && !CalculateHasMatchingCrossPlatformStructure(visitedStructures, member->m_type))
            {
                info->m_has_matching_cross_platform_structure = false;
                return false;
            }
        }

        info->m_has_matching_cross_platform_structure = true;
        return true;
    }
} // namespace

bool CrossPlatformStructurePostProcessor::PostProcess(IDataRepository* repository)
{
    const auto& allInfos = repository->GetAllStructureInformation();

    std::unordered_set<const void*> visitedStructures;
    if (repository->GetWordSize() == OWN_WORD_SIZE)
    {
        for (const auto& info : allInfos)
            info->m_has_matching_cross_platform_structure = true;

        for (const auto& info : allInfos)
        {
            if (info->m_layout_action)
                MarkLayoutStructureAndUsages(visitedStructures, info);
        }
    }
    else
    {
        for (const auto& info : allInfos)
        {
            CalculateHasMatchingCrossPlatformStructure(visitedStructures, info);
        }
    }

    return true;
}
