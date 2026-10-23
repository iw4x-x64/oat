#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

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

    // Where a flow's children are placed along it when they leave room
    enum class LayoutFlowAlign : std::uint8_t
    {
        START,
        CENTER,
        END
    };

    // The axis along which an item scrolls its children
    enum class LayoutScroll : std::uint8_t
    {
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

    // The map an item stands for: its zone's name, the point of the map at the item's center and how many canonical units one map unit
    // spans, 1 if absent
    struct LayoutScene
    {
        std::string m_map;
        std::array<double, 3> m_origin;
        std::optional<double> m_size;
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
        std::optional<LayoutFlowAlign> m_flow_align;
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
        bool m_fit_text = false;

        // The axis the item scrolls its children along, if it does, the scrolling item whose thumb the item is, if it is one, and the local
        // variable that the index of the first of the item's children is kept in, if its children stand for rows of a virtual area
        std::optional<LayoutScroll> m_scroll;
        std::optional<std::string> m_scroll_thumb;
        std::optional<std::string> m_virtual;

        // Whether the item is hidden while it shows its back, and the brightness it keeps facing away from the menu's light if it is lit
        bool m_backface_hidden = false;
        std::optional<double> m_shade;

        // The radius of the item's rounded corners, square if absent
        std::optional<double> m_radius;

        // The map the item stands for, if any, and what stands in it
        std::optional<LayoutScene> m_scene;
        std::optional<std::string> m_subject;

        // The tags of the subject the item is tethered to, the first one the subject has counting, not tethered if empty
        std::vector<std::string> m_tether;

        // The motion of the item, the menu's if absent, its pose in space, the state it enters from, the state it moves to while it has focus
        // and the state it exits to, with the delays in seconds after the menu opens at which it enters and exits
        std::optional<LayoutMotion> m_motion;
        std::optional<double> m_depth;
        std::optional<double> m_rotate_x;
        std::optional<double> m_rotate_y;
        std::optional<double> m_rotate_z;
        std::optional<double> m_scale;
        std::optional<double> m_enter_offset_x;
        std::optional<double> m_enter_offset_y;
        std::optional<double> m_enter_size_x;
        std::optional<double> m_enter_size_y;
        std::optional<double> m_enter_depth;
        std::optional<double> m_enter_rotate_x;
        std::optional<double> m_enter_rotate_y;
        std::optional<double> m_enter_rotate_z;
        std::optional<double> m_enter_scale;
        std::optional<double> m_enter_opacity;
        std::optional<double> m_enter_delay;
        std::optional<double> m_focus_offset_x;
        std::optional<double> m_focus_offset_y;
        std::optional<double> m_focus_size_x;
        std::optional<double> m_focus_size_y;
        std::optional<double> m_focus_depth;
        std::optional<double> m_focus_rotate_x;
        std::optional<double> m_focus_rotate_y;
        std::optional<double> m_focus_rotate_z;
        std::optional<double> m_focus_scale;
        std::optional<double> m_exit_offset_x;
        std::optional<double> m_exit_offset_y;
        std::optional<double> m_exit_size_x;
        std::optional<double> m_exit_size_y;
        std::optional<double> m_exit_depth;
        std::optional<double> m_exit_rotate_x;
        std::optional<double> m_exit_rotate_y;
        std::optional<double> m_exit_rotate_z;
        std::optional<double> m_exit_scale;
        std::optional<double> m_exit_opacity;
        std::optional<double> m_exit_delay;
    };
} // namespace menu
