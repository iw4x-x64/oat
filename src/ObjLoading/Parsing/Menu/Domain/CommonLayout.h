#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>

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

    // The motion that moves an item to where it is placed and to its focus offsets: a preset, a second-order response or a damped spring
    enum class LayoutMotionPreset : std::uint8_t
    {
        NONE,
        SNAPPY,
        SMOOTH,
        SPRINGY
    };

    struct LayoutResponse
    {
        double m_frequency;
        double m_damping;
        double m_response;
    };

    enum class LayoutIntegrator : std::uint8_t
    {
        EULER,
        RK4
    };

    struct LayoutSpring
    {
        double m_stiffness;
        double m_damping;
        LayoutIntegrator m_integrator;
    };

    using LayoutMotion = std::variant<LayoutMotionPreset, LayoutResponse, LayoutSpring>;

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

        // The motion of the item, the menu's if absent, the state it enters from, the state it moves to while it has focus and the state it
        // exits to, with the delays in seconds after the menu opens at which it enters and exits
        std::optional<LayoutMotion> m_motion;
        std::optional<double> m_enter_offset_x;
        std::optional<double> m_enter_offset_y;
        std::optional<double> m_enter_opacity;
        std::optional<double> m_enter_delay;
        std::optional<double> m_focus_offset_x;
        std::optional<double> m_focus_offset_y;
        std::optional<double> m_exit_offset_x;
        std::optional<double> m_exit_offset_y;
        std::optional<double> m_exit_opacity;
        std::optional<double> m_exit_delay;
    };
} // namespace menu
