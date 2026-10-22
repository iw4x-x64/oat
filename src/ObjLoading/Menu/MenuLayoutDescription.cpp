#include "MenuLayoutDescription.h"

#include <cassert>
#include <cctype>
#include <format>
#include <sstream>

using namespace menu;

namespace
{
    const char* SpaceName(const LayoutSpace space)
    {
        switch (space)
        {
        case LayoutSpace::PROTECTED:
            return "protected";
        case LayoutSpace::FULL:
            return "full";
        case LayoutSpace::SAFE:
            return "safe";
        }

        assert(false);
        return "";
    }

    const char* AnchorName(const LayoutAnchor anchor, const bool horizontal)
    {
        switch (anchor)
        {
        case LayoutAnchor::START:
            return horizontal ? "left" : "top";
        case LayoutAnchor::CENTER:
            return "center";
        case LayoutAnchor::END:
            return horizontal ? "right" : "bottom";
        case LayoutAnchor::STRETCH:
            return "stretch";
        }

        assert(false);
        return "";
    }

    const char* FlowName(const LayoutFlow flow)
    {
        switch (flow)
        {
        case LayoutFlow::NONE:
            return "none";
        case LayoutFlow::HORIZONTAL:
            return "horizontal";
        case LayoutFlow::VERTICAL:
            return "vertical";
        }

        assert(false);
        return "";
    }

    const char* FitName(const LayoutFit fit)
    {
        switch (fit)
        {
        case LayoutFit::STRETCH:
            return "stretch";
        case LayoutFit::CONTAIN:
            return "contain";
        case LayoutFit::COVER:
            return "cover";
        case LayoutFit::NATIVE:
            return "native";
        case LayoutFit::TILED:
            return "tiled";
        }

        assert(false);
        return "";
    }

    const char* MotionPresetName(const LayoutMotionPreset preset)
    {
        switch (preset)
        {
        case LayoutMotionPreset::NONE:
            return "none";
        case LayoutMotionPreset::SNAPPY:
            return "snappy";
        case LayoutMotionPreset::SMOOTH:
            return "smooth";
        case LayoutMotionPreset::SPRINGY:
            return "springy";
        }

        assert(false);
        return "";
    }

    const char* IntegratorName(const LayoutIntegrator integrator)
    {
        switch (integrator)
        {
        case LayoutIntegrator::EULER:
            return "euler";
        case LayoutIntegrator::RK4:
            return "rk4";
        }

        assert(false);
        return "";
    }

    std::string MotionStatement(const LayoutMotion& motion)
    {
        if (const auto* preset = std::get_if<LayoutMotionPreset>(&motion))
            return std::format("motion {}\n", MotionPresetName(*preset));

        if (const auto* response = std::get_if<LayoutResponse>(&motion))
            return std::format("motion response {} {} {}\n", response->m_frequency, response->m_damping, response->m_response);

        const auto& spring = std::get<LayoutSpring>(motion);
        return std::format("motion spring {} {} {}\n", spring.m_stiffness, spring.m_damping, IntegratorName(spring.m_integrator));
    }

    std::string Quoted(const std::string& value)
    {
        std::string result("\"");
        for (const auto c : value)
        {
            if (c == '"' || c == '\\')
                result += '\\';

            result += c;
        }
        result += '"';

        return result;
    }

    class DescriptionWriter
    {
    public:
        explicit DescriptionWriter(std::ostream& stream)
            : m_stream(stream)
        {
        }

        void WriteMenu(const CommonMenuDef& menu)
        {
            assert(menu.m_layout_version);

            m_stream << std::format("layoutVersion {}\n", *menu.m_layout_version);
            m_stream << std::format("menu {} {}\n", Quoted(menu.m_name), menu.m_items.size());

            if (menu.m_layout_space)
                m_stream << std::format("layoutSpace {}\n", SpaceName(*menu.m_layout_space));

            if (menu.m_layout_motion)
                m_stream << MotionStatement(*menu.m_layout_motion);

            if (menu.m_layout_perspective)
                m_stream << std::format("perspective {}\n", *menu.m_layout_perspective);

            if (menu.m_layout_light)
                m_stream << std::format("light {} {} {}\n", (*menu.m_layout_light)[0], (*menu.m_layout_light)[1], (*menu.m_layout_light)[2]);

            for (auto i = 0uz; i < menu.m_items.size(); i++)
                WriteItem(i, *menu.m_items[i]);
        }

    private:
        void WriteItem(const std::size_t index, const CommonItemDef& item)
        {
            const auto& layout = item.m_layout;

            m_stream << std::format("\nitem {} {}\n", index, Quoted(item.m_name));

            if (layout.m_space)
                m_stream << std::format("layoutSpace {}\n", SpaceName(*layout.m_space));

            if (layout.m_parent)
                m_stream << std::format("parent {}\n", Quoted(*layout.m_parent));

            WriteProperties(layout.m_normal, "");

            if (layout.m_clip_children)
                m_stream << "clipChildren\n";

            if (layout.m_backface_hidden)
                m_stream << "backfaceHidden\n";

            if (layout.m_shade)
                m_stream << std::format("shade {}\n", *layout.m_shade);

            if (layout.m_compact_below)
                m_stream << std::format("compactBelow {}\n", *layout.m_compact_below);

            WriteProperties(layout.m_compact, "compact");

            if (layout.m_motion)
                m_stream << MotionStatement(*layout.m_motion);

            const std::pair<const char*, const std::optional<double>&> states[]{
                {"depth",        layout.m_depth         },
                {"rotateX",      layout.m_rotate_x      },
                {"rotateY",      layout.m_rotate_y      },
                {"rotateZ",      layout.m_rotate_z      },
                {"scale",        layout.m_scale         },
                {"enterOffsetX", layout.m_enter_offset_x},
                {"enterOffsetY", layout.m_enter_offset_y},
                {"enterSizeX",   layout.m_enter_size_x  },
                {"enterSizeY",   layout.m_enter_size_y  },
                {"enterDepth",   layout.m_enter_depth   },
                {"enterRotateX", layout.m_enter_rotate_x},
                {"enterRotateY", layout.m_enter_rotate_y},
                {"enterRotateZ", layout.m_enter_rotate_z},
                {"enterScale",   layout.m_enter_scale   },
                {"enterOpacity", layout.m_enter_opacity },
                {"enterDelay",   layout.m_enter_delay   },
                {"focusOffsetX", layout.m_focus_offset_x},
                {"focusOffsetY", layout.m_focus_offset_y},
                {"focusDepth",   layout.m_focus_depth   },
                {"focusRotateX", layout.m_focus_rotate_x},
                {"focusRotateY", layout.m_focus_rotate_y},
                {"focusRotateZ", layout.m_focus_rotate_z},
                {"focusScale",   layout.m_focus_scale   },
                {"exitOffsetX",  layout.m_exit_offset_x },
                {"exitOffsetY",  layout.m_exit_offset_y },
                {"exitSizeX",    layout.m_exit_size_x   },
                {"exitSizeY",    layout.m_exit_size_y   },
                {"exitDepth",    layout.m_exit_depth    },
                {"exitRotateX",  layout.m_exit_rotate_x },
                {"exitRotateY",  layout.m_exit_rotate_y },
                {"exitRotateZ",  layout.m_exit_rotate_z },
                {"exitScale",    layout.m_exit_scale    },
                {"exitOpacity",  layout.m_exit_opacity  },
                {"exitDelay",    layout.m_exit_delay    },
            };

            for (const auto& [property, value] : states)
            {
                if (value)
                    m_stream << std::format("{} {}\n", property, *value);
            }
        }

        void WriteProperties(const CommonLayoutProperties& properties, const std::string& prefix)
        {
            const auto name = [&prefix](const std::string& property)
            {
                if (prefix.empty())
                    return property;

                return std::format("{}{}{}", prefix, static_cast<char>(std::toupper(static_cast<unsigned char>(property[0]))), property.substr(1));
            };

            const auto writeNumber = [this, &name](const std::optional<double>& value, const char* property)
            {
                if (value)
                    m_stream << std::format("{} {}\n", name(property), *value);
            };

            const auto writeLength = [this, &name](const std::optional<LayoutLength>& length, const char* fixed, const char* fraction)
            {
                if (length)
                    m_stream << std::format("{} {}\n", name(length->m_fraction ? fraction : fixed), length->m_value);
            };

            if (properties.m_horizontal_anchor)
            {
                assert(properties.m_vertical_anchor);
                m_stream << std::format(
                    "{} {} {}\n", name("anchor"), AnchorName(*properties.m_horizontal_anchor, true), AnchorName(*properties.m_vertical_anchor, false));
            }

            writeNumber(properties.m_offset_x, "offsetX");
            writeNumber(properties.m_offset_y, "offsetY");
            writeLength(properties.m_width, "width", "widthPercent");
            writeLength(properties.m_height, "height", "heightPercent");
            writeNumber(properties.m_min_width, "minWidth");
            writeNumber(properties.m_max_width, "maxWidth");
            writeNumber(properties.m_min_height, "minHeight");
            writeNumber(properties.m_max_height, "maxHeight");

            if (properties.m_aspect_ratio)
                m_stream << std::format("{} {} {}\n", name("aspectRatio"), properties.m_aspect_ratio->m_width, properties.m_aspect_ratio->m_height);

            if (properties.m_padding)
            {
                const auto& padding = *properties.m_padding;
                m_stream << std::format("{} {} {} {} {}\n", name("padding"), padding.m_left, padding.m_top, padding.m_right, padding.m_bottom);
            }

            if (properties.m_flow)
                m_stream << std::format("{} {}\n", name("flow"), FlowName(*properties.m_flow));

            writeNumber(properties.m_item_spacing, "itemSpacing");

            if (properties.m_wrap)
                m_stream << std::format("{} {}\n", name("wrap"), *properties.m_wrap ? 1 : 0);

            if (properties.m_fit)
                m_stream << std::format("{} {}\n", name("fit"), FitName(*properties.m_fit));
        }

        std::ostream& m_stream;
    };
} // namespace

namespace menu
{
    std::string CreateLayoutDescription(const CommonMenuDef& menu)
    {
        std::ostringstream ss;
        DescriptionWriter writer(ss);
        writer.WriteMenu(menu);

        return ss.str();
    }

    std::string GetLayoutDescriptionName(const std::string& menuName)
    {
        return std::format("layout/{}", menuName);
    }
} // namespace menu
