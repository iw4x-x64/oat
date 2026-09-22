#pragma once

namespace menu
{
    enum class FeatureLevel
    {
        IW4,
        IW4MS,
        IW5
    };

    constexpr bool UsesIw4MenuGrammar(const FeatureLevel featureLevel)
    {
        return featureLevel == FeatureLevel::IW4 || featureLevel == FeatureLevel::IW4MS;
    }
} // namespace menu
