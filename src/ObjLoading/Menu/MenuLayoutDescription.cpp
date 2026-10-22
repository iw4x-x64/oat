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

            if (layout.m_compact_below)
                m_stream << std::format("compactBelow {}\n", *layout.m_compact_below);

            WriteProperties(layout.m_compact, "compact");
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
