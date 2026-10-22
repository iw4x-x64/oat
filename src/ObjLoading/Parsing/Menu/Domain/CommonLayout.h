#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace menu
{
    // The layout properties of a menu that declares a layoutVersion, which the client resolves against the display at run time.
    // Lengths are in canonical units, that is in pixels of a 1920x1080 display.

    enum class LayoutSpace : std::uint8_t
    {
        PROTECTED,
        FULL,
        SAFE
    };

    enum class LayoutAnchor : std::uint8_t
    {
        START,
        CENTER,
        END,
        STRETCH
    };

    enum class LayoutFlow : std::uint8_t
    {
        NONE,
        HORIZONTAL,
        VERTICAL
    };

    enum class LayoutFit : std::uint8_t
    {
        STRETCH,
        CONTAIN,
        COVER,
        NATIVE,
        TILED
    };

    struct LayoutLength
    {
        double m_value;
        bool m_fraction;
    };

    struct LayoutInsets
    {
        double m_left;
        double m_top;
        double m_right;
        double m_bottom;
    };

    struct LayoutRatio
    {
        double m_width;
        double m_height;
    };

    class CommonLayoutProperties
    {
    public:
        std::optional<LayoutAnchor> m_horizontal_anchor;
        std::optional<LayoutAnchor> m_vertical_anchor;
        std::optional<double> m_offset_x;
        std::optional<double> m_offset_y;
        std::optional<LayoutLength> m_width;
        std::optional<LayoutLength> m_height;
        std::optional<double> m_min_width;
        std::optional<double> m_max_width;
        std::optional<double> m_min_height;
        std::optional<double> m_max_height;
        std::optional<LayoutRatio> m_aspect_ratio;
        std::optional<LayoutInsets> m_padding;
        std::optional<LayoutFlow> m_flow;
        std::optional<double> m_item_spacing;
        std::optional<bool> m_wrap;
        std::optional<LayoutFit> m_fit;

        [[nodiscard]] bool IsEmpty() const;
    };

    class CommonItemLayout
    {
    public:
        std::optional<LayoutSpace> m_space;
        std::optional<std::string> m_parent;
        CommonLayoutProperties m_normal;
        CommonLayoutProperties m_compact;
        std::optional<double> m_compact_below;
        bool m_clip_children = false;
    };
} // namespace menu
