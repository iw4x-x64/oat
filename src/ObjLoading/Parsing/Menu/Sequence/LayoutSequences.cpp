#include "LayoutSequences.h"

#include "Generic/GenericFloatingPointPropertySequence.h"
#include "Generic/GenericIntPropertySequence.h"
#include "Generic/GenericKeywordPropertySequence.h"
#include "Generic/GenericStringPropertySequence.h"
#include "Parsing/Menu/Matcher/MenuExpressionMatchers.h"
#include "Parsing/Menu/Matcher/MenuMatcherFactory.h"
#include "Utils/StringUtils.h"

#include <cassert>
#include <cctype>
#include <format>
#include <functional>
#include <span>
#include <sstream>
#include <string>
#include <utility>

using namespace menu;

namespace
{
    constexpr auto SUPPORTED_LAYOUT_VERSION = 1;

    template<typename T> struct Spelling
    {
        const char* m_name;
        T m_value;
    };

    constexpr Spelling<LayoutSpace> SPACE_SPELLINGS[]{
        {"protected", LayoutSpace::PROTECTED},
        {"full",      LayoutSpace::FULL     },
        {"safe",      LayoutSpace::SAFE     },
    };

    constexpr Spelling<LayoutAnchor> HORIZONTAL_ANCHOR_SPELLINGS[]{
        {"left",    LayoutAnchor::START  },
        {"center",  LayoutAnchor::CENTER },
        {"right",   LayoutAnchor::END    },
        {"stretch", LayoutAnchor::STRETCH},
    };

    constexpr Spelling<LayoutAnchor> VERTICAL_ANCHOR_SPELLINGS[]{
        {"top",     LayoutAnchor::START  },
        {"center",  LayoutAnchor::CENTER },
        {"bottom",  LayoutAnchor::END    },
        {"stretch", LayoutAnchor::STRETCH},
    };

    constexpr Spelling<LayoutFlow> FLOW_SPELLINGS[]{
        {"none",       LayoutFlow::NONE      },
        {"horizontal", LayoutFlow::HORIZONTAL},
        {"vertical",   LayoutFlow::VERTICAL  },
    };

    constexpr Spelling<LayoutFit> FIT_SPELLINGS[]{
        {"stretch", LayoutFit::STRETCH},
        {"contain", LayoutFit::CONTAIN},
        {"cover",   LayoutFit::COVER  },
        {"native",  LayoutFit::NATIVE },
        {"tiled",   LayoutFit::TILED  },
    };

    constexpr Spelling<LayoutMotionPreset> MOTION_PRESET_SPELLINGS[]{
        {"none",    LayoutMotionPreset::NONE   },
        {"snappy",  LayoutMotionPreset::SNAPPY },
        {"smooth",  LayoutMotionPreset::SMOOTH },
        {"springy", LayoutMotionPreset::SPRINGY},
    };

    constexpr Spelling<LayoutIntegrator> INTEGRATOR_SPELLINGS[]{
        {"euler", LayoutIntegrator::EULER},
        {"rk4",   LayoutIntegrator::RK4  },
    };

    template<typename T> T ParseSpelling(const std::span<const Spelling<T>> spellings, const SimpleParserValue& token, const char* what)
    {
        const auto& value = MenuMatcherFactory::TokenTextValue(token);
        for (const auto& spelling : spellings)
        {
            if (utils::StringEqualsIgnoreCase(value, spelling.m_name))
                return spelling.m_value;
        }

        throw ParsingException(token.GetPos(), std::format("Invalid {} \"{}\"", what, value));
    }

    // The two sets of layout properties of an item: the normal one and the compact one, whose keywords are the normal ones with the compact prefix
    class PropertySet
    {
    public:
        const char* m_prefix;
        CommonLayoutProperties CommonItemLayout::* m_properties;

        [[nodiscard]] std::string Name(const std::string& name) const
        {
            if (*m_prefix == '\0')
                return name;

            auto result = std::string(m_prefix);
            result += static_cast<char>(std::toupper(static_cast<unsigned char>(name[0])));
            result += name.substr(1);

            return result;
        }
    };

    constexpr PropertySet PROPERTY_SETS[]{
        {"",        &CommonItemLayout::m_normal },
        {"compact", &CommonItemLayout::m_compact},
    };

    constexpr PropertySet NORMAL_PROPERTY_SET = PROPERTY_SETS[0];

    CommonItemLayout& CurrentItemLayout(const MenuFileParserState* state, const TokenPos& pos)
    {
        LayoutSequences::EnsureIsLayoutMenu(state, pos);
        assert(state->m_current_item);

        return state->m_current_item->m_layout;
    }

    CommonLayoutProperties& CurrentProperties(const MenuFileParserState* state, const TokenPos& pos, const PropertySet& set)
    {
        return CurrentItemLayout(state, pos).*set.m_properties;
    }

    template<typename T> void SetOnce(std::optional<T>& slot, T value, const TokenPos& pos, const std::string& name)
    {
        if (slot)
            throw ParsingException(pos, std::format("Layout property \"{}\" has already been set", name));

        slot = std::move(value);
    }

    double EnsureNonNegative(const double value, const TokenPos& pos, const std::string& name)
    {
        if (value < 0)
            throw ParsingException(pos, std::format("Layout property \"{}\" must not be negative", name));

        return value;
    }

    double EnsurePositive(const double value, const TokenPos& pos, const std::string& name)
    {
        if (value <= 0)
            throw ParsingException(pos, std::format("Layout property \"{}\" must be positive", name));

        return value;
    }

    // A stretched axis takes its size from the parent, which leaves no room for a size of its own
    void EnsureAxisNotStretched(const std::optional<LayoutAnchor>& anchor, const TokenPos& pos, const std::string& name)
    {
        if (anchor == LayoutAnchor::STRETCH)
            throw ParsingException(pos, std::format("Layout property \"{}\" conflicts with a stretch anchor", name));
    }

    void SetLength(const PropertySet& set,
                   std::optional<LayoutLength> CommonLayoutProperties::* member,
                   std::optional<LayoutAnchor> CommonLayoutProperties::* anchor,
                   const MenuFileParserState* state,
                   const TokenPos& pos,
                   const std::string& name,
                   const LayoutLength length)
    {
        auto& properties = CurrentProperties(state, pos, set);

        EnsureAxisNotStretched(properties.*anchor, pos, name);
        SetOnce(properties.*member, length, pos, name);
    }

    // Matches a keyword followed by a fixed number of identifiers
    class SequenceIdentifiers final : public MenuFileParser::sequence_t
    {
    public:
        using callback_t = std::function<void(MenuFileParserState* state, const TokenPos& pos, std::span<const SimpleParserValue* const> values)>;

    private:
        static constexpr auto CAPTURE_FIRST_TOKEN = 1;
        static constexpr auto CAPTURE_VALUE = 2;

        const callback_t m_set_callback;

    public:
        SequenceIdentifiers(std::string keywordName, const std::size_t count, callback_t setCallback)
            : m_set_callback(std::move(setCallback))
        {
            const MenuMatcherFactory create(this);

            std::vector<std::unique_ptr<AbstractMatcher<SimpleParserValue>>> matchers;
            matchers.emplace_back(create.KeywordIgnoreCase(std::move(keywordName)).Capture(CAPTURE_FIRST_TOKEN));
            for (auto i = 0uz; i < count; i++)
                matchers.emplace_back(create.Identifier().Capture(CAPTURE_VALUE));

            AddMatchers(create.And(std::move(matchers)));
        }

    protected:
        void ProcessMatch(MenuFileParserState* state, SequenceResult<SimpleParserValue>& result) const override
        {
            const auto& firstToken = result.NextCapture(CAPTURE_FIRST_TOKEN);

            std::vector<const SimpleParserValue*> values;
            while (result.HasNextCapture(CAPTURE_VALUE))
                values.emplace_back(&result.NextCapture(CAPTURE_VALUE));

            m_set_callback(state, firstToken.GetPos(), values);
        }
    };

    // Matches a keyword followed by one or more numeric expressions
    class SequenceNumbers final : public MenuFileParser::sequence_t
    {
    public:
        using callback_t = std::function<void(MenuFileParserState* state, const TokenPos& pos, std::span<const double> values)>;

    private:
        static constexpr auto CAPTURE_FIRST_TOKEN = 1;

        const callback_t m_set_callback;

    public:
        SequenceNumbers(std::string keywordName, callback_t setCallback)
            : m_set_callback(std::move(setCallback))
        {
            const MenuMatcherFactory create(this);

            AddLabeledMatchers(MenuExpressionMatchers().Expression(this), MenuExpressionMatchers::LABEL_EXPRESSION);
            AddMatchers({
                create.KeywordIgnoreCase(std::move(keywordName)).Capture(CAPTURE_FIRST_TOKEN),
                create.NumericExpression(),
                create.OptionalLoop(create.NumericExpression()),
            });
        }

    protected:
        void ProcessMatch(MenuFileParserState* state, SequenceResult<SimpleParserValue>& result) const override
        {
            const auto& firstToken = result.NextCapture(CAPTURE_FIRST_TOKEN);

            std::vector<double> values;
            while (result.PeekTag() != AbstractMatcher<SimpleParserValue>::NO_ID)
                values.emplace_back(MenuMatcherFactory::TokenNumericExpressionValue(state, result));

            m_set_callback(state, firstToken.GetPos(), values);
        }
    };

    // Matches scene followed by the map's name and one or more numeric expressions
    class SequenceScene final : public MenuFileParser::sequence_t
    {
        static constexpr auto CAPTURE_FIRST_TOKEN = 1;
        static constexpr auto CAPTURE_MAP = 2;

    public:
        SequenceScene()
        {
            const MenuMatcherFactory create(this);

            AddLabeledMatchers(MenuExpressionMatchers().Expression(this), MenuExpressionMatchers::LABEL_EXPRESSION);
            AddMatchers({
                create.KeywordIgnoreCase("scene").Capture(CAPTURE_FIRST_TOKEN),
                create.Text().Capture(CAPTURE_MAP),
                create.NumericExpression(),
                create.OptionalLoop(create.NumericExpression()),
            });
        }

    protected:
        void ProcessMatch(MenuFileParserState* state, SequenceResult<SimpleParserValue>& result) const override
        {
            const auto& firstToken = result.NextCapture(CAPTURE_FIRST_TOKEN);
            const auto& mapToken = result.NextCapture(CAPTURE_MAP);
            const auto& pos = firstToken.GetPos();

            std::vector<double> values;
            while (result.PeekTag() != AbstractMatcher<SimpleParserValue>::NO_ID)
                values.emplace_back(MenuMatcherFactory::TokenNumericExpressionValue(state, result));

            if (values.size() != 3u && values.size() != 4u)
                throw ParsingException(pos, std::format("Layout property \"scene\" takes a map and 3 or 4 values instead of {}", values.size()));

            const auto map = MenuMatcherFactory::TokenTextValue(mapToken);
            if (map.empty())
                throw ParsingException(mapToken.GetPos(), "Layout property \"scene\" must name a map");

            LayoutScene scene{
                .m_map = map, .m_origin = {values[0], values[1], values[2]},
                     .m_size = std::nullopt
            };
            if (values.size() == 4u)
                scene.m_size = EnsurePositive(values[3], pos, "scene size");

            SetOnce(CurrentItemLayout(state, pos).m_scene, std::move(scene), pos, "scene");
        }
    };

    // Matches tether followed by the tag names, separated by spaces in one string since the lexer joins adjacent strings
    class SequenceTether final : public MenuFileParser::sequence_t
    {
        static constexpr auto CAPTURE_FIRST_TOKEN = 1;
        static constexpr auto CAPTURE_TAGS = 2;

    public:
        SequenceTether()
        {
            const MenuMatcherFactory create(this);

            AddMatchers({
                create.KeywordIgnoreCase("tether").Capture(CAPTURE_FIRST_TOKEN),
                create.Text().Capture(CAPTURE_TAGS),
            });
        }

    protected:
        void ProcessMatch(MenuFileParserState* state, SequenceResult<SimpleParserValue>& result) const override
        {
            const auto& firstToken = result.NextCapture(CAPTURE_FIRST_TOKEN);
            const auto& tagsToken = result.NextCapture(CAPTURE_TAGS);
            const auto& pos = firstToken.GetPos();

            std::vector<std::string> tags;
            std::istringstream stream(MenuMatcherFactory::TokenTextValue(tagsToken));
            for (std::string tag; stream >> tag;)
                tags.emplace_back(std::move(tag));

            if (tags.empty())
                throw ParsingException(tagsToken.GetPos(), "Layout property \"tether\" must name tags");

            auto& layout = CurrentItemLayout(state, pos);
            if (!layout.m_tether.empty())
                throw ParsingException(pos, "Layout property \"tether\" has already been set");

            layout.m_tether = std::move(tags);
        }
    };

    // Matches wrap with an optional 0 or 1, which is how the compact form turns the normal one off
    class SequenceWrap final : public MenuFileParser::sequence_t
    {
        static constexpr auto CAPTURE_FIRST_TOKEN = 1;
        static constexpr auto CAPTURE_VALUE = 2;

        PropertySet m_set;

    public:
        explicit SequenceWrap(const PropertySet& set)
            : m_set(set)
        {
            const MenuMatcherFactory create(this);

            AddMatchers({
                create.KeywordIgnoreCase(set.Name("wrap")).Capture(CAPTURE_FIRST_TOKEN),
                create.Optional(create.Integer().Capture(CAPTURE_VALUE)),
            });
        }

    protected:
        void ProcessMatch(MenuFileParserState* state, SequenceResult<SimpleParserValue>& result) const override
        {
            const auto name = m_set.Name("wrap");
            const auto& firstToken = result.NextCapture(CAPTURE_FIRST_TOKEN);

            auto value = true;
            if (result.HasNextCapture(CAPTURE_VALUE))
            {
                const auto& valueToken = result.NextCapture(CAPTURE_VALUE);
                const auto intValue = valueToken.IntegerValue();
                if (intValue != 0 && intValue != 1)
                    throw ParsingException(valueToken.GetPos(), std::format("Layout property \"{}\" must be 0 or 1", name));

                value = intValue == 1;
            }

            SetOnce(CurrentProperties(state, firstToken.GetPos(), m_set).m_wrap, value, firstToken.GetPos(), name);
        }
    };

    // Matches motion followed by a preset, by response and its frequency, damping and response, or by spring and its stiffness, damping
    // and integrator
    class SequenceMotion final : public MenuFileParser::sequence_t
    {
    public:
        using callback_t = std::function<void(MenuFileParserState* state, const TokenPos& pos, LayoutMotion motion)>;

    private:
        static constexpr auto TAG_PARAMETERS = 1;
        static constexpr auto TAG_RESPONSE = 2;
        static constexpr auto CAPTURE_FIRST_TOKEN = 1;
        static constexpr auto CAPTURE_KIND = 2;
        static constexpr auto CAPTURE_INTEGRATOR = 3;

        const callback_t m_set_callback;

    public:
        explicit SequenceMotion(callback_t setCallback)
            : m_set_callback(std::move(setCallback))
        {
            const MenuMatcherFactory create(this);

            AddLabeledMatchers(MenuExpressionMatchers().Expression(this), MenuExpressionMatchers::LABEL_EXPRESSION);
            AddMatchers({
                create.KeywordIgnoreCase("motion").Capture(CAPTURE_FIRST_TOKEN),
                create.Identifier().Capture(CAPTURE_KIND),
                create.Optional(create
                                    .And({
                                        create.NumericExpression(),
                                        create.NumericExpression(),
                                        create.Or({
                                            create.NumericExpression().Tag(TAG_RESPONSE),
                                            create.Identifier().Capture(CAPTURE_INTEGRATOR),
                                        }),
                                    })
                                    .Tag(TAG_PARAMETERS)),
            });
        }

    protected:
        void ProcessMatch(MenuFileParserState* state, SequenceResult<SimpleParserValue>& result) const override
        {
            const auto& firstToken = result.NextCapture(CAPTURE_FIRST_TOKEN);
            const auto& kindToken = result.NextCapture(CAPTURE_KIND);
            const auto& kind = MenuMatcherFactory::TokenTextValue(kindToken);

            const auto response = utils::StringEqualsIgnoreCase(kind, "response");
            const auto spring = utils::StringEqualsIgnoreCase(kind, "spring");

            if (result.PeekAndRemoveIfTag(TAG_PARAMETERS) != TAG_PARAMETERS)
            {
                if (response || spring)
                    throw ParsingException(kindToken.GetPos(),
                                           std::format("Motion \"{}\" takes {}", kind, response ? "3 values" : "2 values and an integrator"));

                m_set_callback(state, firstToken.GetPos(), ParseSpelling<LayoutMotionPreset>(MOTION_PRESET_SPELLINGS, kindToken, "motion"));
                return;
            }

            const auto first = MenuMatcherFactory::TokenNumericExpressionValue(state, result);
            const auto second = MenuMatcherFactory::TokenNumericExpressionValue(state, result);

            if (result.PeekAndRemoveIfTag(TAG_RESPONSE) == TAG_RESPONSE)
            {
                const auto third = MenuMatcherFactory::TokenNumericExpressionValue(state, result);
                if (!response)
                    throw ParsingException(kindToken.GetPos(), std::format("Motion \"{}\" does not take 3 values", kind));

                m_set_callback(state,
                               firstToken.GetPos(),
                               LayoutResponse{EnsurePositive(first, firstToken.GetPos(), "motion frequency"),
                                              EnsureNonNegative(second, firstToken.GetPos(), "motion damping"),
                                              third});
                return;
            }

            const auto& integratorToken = result.NextCapture(CAPTURE_INTEGRATOR);
            if (!spring)
                throw ParsingException(kindToken.GetPos(), std::format("Motion \"{}\" does not take an integrator", kind));

            m_set_callback(state,
                           firstToken.GetPos(),
                           LayoutSpring{EnsurePositive(first, firstToken.GetPos(), "spring stiffness"),
                                        EnsureNonNegative(second, firstToken.GetPos(), "spring damping"),
                                        ParseSpelling<LayoutIntegrator>(INTEGRATOR_SPELLINGS, integratorToken, "integrator")});
        }
    };

    void AddPropertySetSequences(std::vector<std::unique_ptr<MenuFileParser::sequence_t>>& sequences, const PropertySet& set)
    {
        sequences.emplace_back(std::make_unique<SequenceIdentifiers>(
            set.Name("anchor"),
            2u,
            [set](const MenuFileParserState* state, const TokenPos& pos, const std::span<const SimpleParserValue* const> values)
            {
                const auto name = set.Name("anchor");
                auto& properties = CurrentProperties(state, pos, set);

                const auto horizontal = ParseSpelling<LayoutAnchor>(HORIZONTAL_ANCHOR_SPELLINGS, *values[0], "horizontal anchor");
                const auto vertical = ParseSpelling<LayoutAnchor>(VERTICAL_ANCHOR_SPELLINGS, *values[1], "vertical anchor");

                if (horizontal == LayoutAnchor::STRETCH && properties.m_width)
                    throw ParsingException(pos, std::format("Layout property \"{}\" conflicts with the width", name));
                if (vertical == LayoutAnchor::STRETCH && properties.m_height)
                    throw ParsingException(pos, std::format("Layout property \"{}\" conflicts with the height", name));

                SetOnce(properties.m_horizontal_anchor, horizontal, pos, name);
                properties.m_vertical_anchor = vertical;
            }));

        // Offsets may be negative, every other single number is a size or a distance
        const std::pair<const char*, std::optional<double> CommonLayoutProperties::*> offsets[]{
            {"offsetX", &CommonLayoutProperties::m_offset_x},
            {"offsetY", &CommonLayoutProperties::m_offset_y},
        };

        for (const auto& [keyword, member] : offsets)
        {
            const auto name = set.Name(keyword);
            sequences.emplace_back(std::make_unique<GenericFloatingPointPropertySequence>(
                name,
                [set, member, name](const MenuFileParserState* state, const TokenPos& pos, const double value)
                {
                    SetOnce(CurrentProperties(state, pos, set).*member, value, pos, name);
                }));
        }

        const std::pair<const char*, std::optional<double> CommonLayoutProperties::*> extents[]{
            {"minWidth",    &CommonLayoutProperties::m_min_width   },
            {"maxWidth",    &CommonLayoutProperties::m_max_width   },
            {"minHeight",   &CommonLayoutProperties::m_min_height  },
            {"maxHeight",   &CommonLayoutProperties::m_max_height  },
            {"itemSpacing", &CommonLayoutProperties::m_item_spacing},
        };

        for (const auto& [keyword, member] : extents)
        {
            const auto name = set.Name(keyword);
            sequences.emplace_back(std::make_unique<GenericFloatingPointPropertySequence>(
                name,
                [set, member, name](const MenuFileParserState* state, const TokenPos& pos, const double value)
                {
                    SetOnce(CurrentProperties(state, pos, set).*member, EnsureNonNegative(value, pos, name), pos, name);
                }));
        }

        // width and widthPercent are two forms of the same property, as are height and heightPercent
        struct LengthKeyword
        {
            const char* m_keyword;
            bool m_fraction;
            std::optional<LayoutLength> CommonLayoutProperties::* m_member;
            std::optional<LayoutAnchor> CommonLayoutProperties::* m_anchor;
        };

        const LengthKeyword lengths[]{
            {"width",         false, &CommonLayoutProperties::m_width,  &CommonLayoutProperties::m_horizontal_anchor},
            {"height",        false, &CommonLayoutProperties::m_height, &CommonLayoutProperties::m_vertical_anchor  },
            {"widthPercent",  true,  &CommonLayoutProperties::m_width,  &CommonLayoutProperties::m_horizontal_anchor},
            {"heightPercent", true,  &CommonLayoutProperties::m_height, &CommonLayoutProperties::m_vertical_anchor  },
        };

        for (const auto& length : lengths)
        {
            const auto name = set.Name(length.m_keyword);
            sequences.emplace_back(std::make_unique<GenericFloatingPointPropertySequence>(
                name,
                [set, length, name](const MenuFileParserState* state, const TokenPos& pos, const double value)
                {
                    SetLength(set, length.m_member, length.m_anchor, state, pos, name, LayoutLength{EnsureNonNegative(value, pos, name), length.m_fraction});
                }));
        }

        sequences.emplace_back(std::make_unique<SequenceNumbers>(
            set.Name("aspectRatio"),
            [set](const MenuFileParserState* state, const TokenPos& pos, const std::span<const double> values)
            {
                const auto name = set.Name("aspectRatio");
                if (values.size() != 2u)
                    throw ParsingException(pos, std::format("Layout property \"{}\" takes 2 values instead of {}", name, values.size()));

                SetOnce(CurrentProperties(state, pos, set).m_aspect_ratio,
                        LayoutRatio{EnsurePositive(values[0], pos, name), EnsurePositive(values[1], pos, name)},
                        pos,
                        name);
            }));

        sequences.emplace_back(std::make_unique<SequenceNumbers>(
            set.Name("padding"),
            [set](const MenuFileParserState* state, const TokenPos& pos, const std::span<const double> values)
            {
                const auto name = set.Name("padding");
                for (const auto value : values)
                    EnsureNonNegative(value, pos, name);

                LayoutInsets insets{};
                switch (values.size())
                {
                case 1u:
                    insets = LayoutInsets{values[0], values[0], values[0], values[0]};
                    break;

                case 2u:
                    insets = LayoutInsets{values[0], values[1], values[0], values[1]};
                    break;

                case 4u:
                    insets = LayoutInsets{values[0], values[1], values[2], values[3]};
                    break;

                default:
                    throw ParsingException(pos, std::format("Layout property \"{}\" takes 1, 2 or 4 values instead of {}", name, values.size()));
                }

                SetOnce(CurrentProperties(state, pos, set).m_padding, insets, pos, name);
            }));

        sequences.emplace_back(std::make_unique<SequenceIdentifiers>(
            set.Name("flow"),
            1u,
            [set](const MenuFileParserState* state, const TokenPos& pos, const std::span<const SimpleParserValue* const> values)
            {
                SetOnce(CurrentProperties(state, pos, set).m_flow, ParseSpelling<LayoutFlow>(FLOW_SPELLINGS, *values[0], "flow"), pos, set.Name("flow"));
            }));

        sequences.emplace_back(std::make_unique<SequenceWrap>(set));

        sequences.emplace_back(std::make_unique<SequenceIdentifiers>(
            set.Name("fit"),
            1u,
            [set](const MenuFileParserState* state, const TokenPos& pos, const std::span<const SimpleParserValue* const> values)
            {
                SetOnce(CurrentProperties(state, pos, set).m_fit, ParseSpelling<LayoutFit>(FIT_SPELLINGS, *values[0], "fit"), pos, set.Name("fit"));
            }));
    }
} // namespace

void LayoutSequences::EnsureIsLayoutMenu(const MenuFileParserState* state, const TokenPos& pos)
{
    assert(state->m_current_menu);

    if (!state->m_current_menu->m_layout_version)
        throw ParsingException(pos, "Layout properties require the menu to specify layoutVersion");
}

void LayoutSequences::EnsureIsNotLayoutMenu(const MenuFileParserState* state, const TokenPos& pos, const char* what)
{
    assert(state->m_current_menu);

    if (state->m_current_menu->m_layout_version)
        throw ParsingException(pos, std::format("{} are not supported in layout menus", what));
}

void LayoutSequences::EnsureCompactPropertiesHaveThreshold(const MenuFileParserState* state, const TokenPos& pos)
{
    assert(state->m_current_item);

    const auto& layout = state->m_current_item->m_layout;
    if (!layout.m_compact.IsEmpty() && !layout.m_compact_below)
        throw ParsingException(pos, "Compact layout properties require compactBelow");
}

void LayoutSequences::EnsureExitFollowsEnter(const MenuFileParserState* state, const TokenPos& pos)
{
    assert(state->m_current_item);

    const auto& layout = state->m_current_item->m_layout;
    if (layout.m_exit_delay && *layout.m_exit_delay < layout.m_enter_delay.value_or(0))
        throw ParsingException(pos, "Layout property \"exitDelay\" must not be less than enterDelay");
}

void LayoutSequences::SetRect(const MenuFileParserState* state, const TokenPos& pos, const double x, const double y, const double w, const double h)
{
    const auto& set = NORMAL_PROPERTY_SET;
    auto& properties = CurrentProperties(state, pos, set);

    SetOnce(properties.m_offset_x, x, pos, "offsetX");
    SetOnce(properties.m_offset_y, y, pos, "offsetY");
    SetLength(set,
              &CommonLayoutProperties::m_width,
              &CommonLayoutProperties::m_horizontal_anchor,
              state,
              pos,
              "width",
              LayoutLength{EnsureNonNegative(w, pos, "rect"), false});
    SetLength(set,
              &CommonLayoutProperties::m_height,
              &CommonLayoutProperties::m_vertical_anchor,
              state,
              pos,
              "height",
              LayoutLength{EnsureNonNegative(h, pos, "rect"), false});
}

std::vector<std::unique_ptr<MenuFileParser::sequence_t>> LayoutSequences::CreateMenuSequences()
{
    std::vector<std::unique_ptr<MenuFileParser::sequence_t>> sequences;

    sequences.emplace_back(std::make_unique<GenericIntPropertySequence>(
        "layoutVersion",
        [](const MenuFileParserState* state, const TokenPos& pos, const int value)
        {
            auto& menu = *state->m_current_menu;

            if (menu.m_layout_version)
                throw ParsingException(pos, "layoutVersion has already been set");
            if (value != SUPPORTED_LAYOUT_VERSION)
                throw ParsingException(pos, std::format("Unsupported layoutVersion {}, expected {}", value, SUPPORTED_LAYOUT_VERSION));
            if (!menu.m_items.empty())
                throw ParsingException(pos, "layoutVersion must be specified before the first itemDef");

            menu.m_layout_version = value;
        }));

    sequences.emplace_back(std::make_unique<SequenceIdentifiers>(
        "layoutSpace",
        1u,
        [](const MenuFileParserState* state, const TokenPos& pos, const std::span<const SimpleParserValue* const> values)
        {
            LayoutSequences::EnsureIsLayoutMenu(state, pos);
            SetOnce(state->m_current_menu->m_layout_space, ParseSpelling<LayoutSpace>(SPACE_SPELLINGS, *values[0], "layout space"), pos, "layoutSpace");
        }));

    sequences.emplace_back(std::make_unique<SequenceMotion>(
        [](const MenuFileParserState* state, const TokenPos& pos, LayoutMotion motion)
        {
            LayoutSequences::EnsureIsLayoutMenu(state, pos);
            SetOnce(state->m_current_menu->m_layout_motion, std::move(motion), pos, "motion");
        }));

    sequences.emplace_back(std::make_unique<GenericFloatingPointPropertySequence>(
        "perspective",
        [](const MenuFileParserState* state, const TokenPos& pos, const double value)
        {
            LayoutSequences::EnsureIsLayoutMenu(state, pos);
            SetOnce(state->m_current_menu->m_layout_perspective, EnsurePositive(value, pos, "perspective"), pos, "perspective");
        }));

    // The direction the light comes from, which must have one
    sequences.emplace_back(std::make_unique<SequenceNumbers>(
        "light",
        [](const MenuFileParserState* state, const TokenPos& pos, const std::span<const double> values)
        {
            LayoutSequences::EnsureIsLayoutMenu(state, pos);

            if (values.size() != 3u)
                throw ParsingException(pos, std::format("Layout property \"light\" takes 3 values instead of {}", values.size()));
            if (values[0] == 0 && values[1] == 0 && values[2] == 0)
                throw ParsingException(pos, "Layout property \"light\" must have a direction");

            SetOnce(state->m_current_menu->m_layout_light, std::array{values[0], values[1], values[2]}, pos, "light");
        }));

    return sequences;
}

std::vector<std::unique_ptr<MenuFileParser::sequence_t>> LayoutSequences::CreateItemSequences()
{
    std::vector<std::unique_ptr<MenuFileParser::sequence_t>> sequences;

    sequences.emplace_back(std::make_unique<SequenceIdentifiers>(
        "layoutSpace",
        1u,
        [](const MenuFileParserState* state, const TokenPos& pos, const std::span<const SimpleParserValue* const> values)
        {
            SetOnce(CurrentItemLayout(state, pos).m_space, ParseSpelling<LayoutSpace>(SPACE_SPELLINGS, *values[0], "layout space"), pos, "layoutSpace");
        }));

    sequences.emplace_back(std::make_unique<GenericStringPropertySequence>("parent",
                                                                           [](const MenuFileParserState* state, const TokenPos& pos, const std::string& value)
                                                                           {
                                                                               SetOnce(CurrentItemLayout(state, pos).m_parent, value, pos, "parent");
                                                                           }));

    sequences.emplace_back(std::make_unique<GenericKeywordPropertySequence>("clipChildren",
                                                                            [](const MenuFileParserState* state, const TokenPos& pos)
                                                                            {
                                                                                auto& layout = CurrentItemLayout(state, pos);
                                                                                if (layout.m_clip_children)
                                                                                    throw ParsingException(
                                                                                        pos, "Layout property \"clipChildren\" has already been set");

                                                                                layout.m_clip_children = true;
                                                                            }));

    sequences.emplace_back(std::make_unique<GenericKeywordPropertySequence>("fitText",
                                                                            [](const MenuFileParserState* state, const TokenPos& pos)
                                                                            {
                                                                                auto& layout = CurrentItemLayout(state, pos);
                                                                                if (layout.m_fit_text)
                                                                                    throw ParsingException(pos, "Layout property \"fitText\" has already been set");

                                                                                layout.m_fit_text = true;
                                                                            }));

    sequences.emplace_back(std::make_unique<GenericKeywordPropertySequence>("backfaceHidden",
                                                                            [](const MenuFileParserState* state, const TokenPos& pos)
                                                                            {
                                                                                auto& layout = CurrentItemLayout(state, pos);
                                                                                if (layout.m_backface_hidden)
                                                                                    throw ParsingException(
                                                                                        pos, "Layout property \"backfaceHidden\" has already been set");

                                                                                layout.m_backface_hidden = true;
                                                                            }));

    sequences.emplace_back(std::make_unique<GenericFloatingPointPropertySequence>("shade",
                                                                                  [](const MenuFileParserState* state, const TokenPos& pos, const double value)
                                                                                  {
                                                                                      if (value < 0 || value > 1)
                                                                                          throw ParsingException(
                                                                                              pos, "Layout property \"shade\" must be from 0 to 1");

                                                                                      SetOnce(CurrentItemLayout(state, pos).m_shade, value, pos, "shade");
                                                                                  }));

    sequences.emplace_back(std::make_unique<GenericFloatingPointPropertySequence>("radius",
                                                                                  [](const MenuFileParserState* state, const TokenPos& pos, const double value)
                                                                                  {
                                                                                      if (value < 0)
                                                                                          throw ParsingException(
                                                                                              pos, "Layout property \"radius\" must not be negative");

                                                                                      SetOnce(CurrentItemLayout(state, pos).m_radius, value, pos, "radius");
                                                                                  }));

    sequences.emplace_back(std::make_unique<SequenceScene>());
    sequences.emplace_back(std::make_unique<SequenceTether>());

    sequences.emplace_back(std::make_unique<GenericStringPropertySequence>("subject",
                                                                           [](const MenuFileParserState* state, const TokenPos& pos, const std::string& value)
                                                                           {
                                                                               if (value.empty())
                                                                                   throw ParsingException(pos,
                                                                                                          "Layout property \"subject\" must name a subject");

                                                                               SetOnce(CurrentItemLayout(state, pos).m_subject, value, pos, "subject");
                                                                           }));

    sequences.emplace_back(std::make_unique<GenericFloatingPointPropertySequence>(
        "compactBelow",
        [](const MenuFileParserState* state, const TokenPos& pos, const double value)
        {
            SetOnce(CurrentItemLayout(state, pos).m_compact_below, EnsurePositive(value, pos, "compactBelow"), pos, "compactBelow");
        }));

    sequences.emplace_back(std::make_unique<SequenceMotion>(
        [](const MenuFileParserState* state, const TokenPos& pos, LayoutMotion motion)
        {
            SetOnce(CurrentItemLayout(state, pos).m_motion, std::move(motion), pos, "motion");
        }));

    // The pose and the enter, focus and exit states: offsets of the position or the size, depths, rotations and scales, which may be anything,
    // opacities, which go from 0 to 1, and delays, which are never negative
    enum class StateValue : std::uint8_t
    {
        OFFSET,
        OPACITY,
        DELAY
    };

    struct StateProperty
    {
        const char* m_keyword;
        std::optional<double> CommonItemLayout::* m_member;
        StateValue m_value;
    };

    constexpr StateProperty states[]{
        {"depth",        &CommonItemLayout::m_depth,          StateValue::OFFSET },
        {"rotateX",      &CommonItemLayout::m_rotate_x,       StateValue::OFFSET },
        {"rotateY",      &CommonItemLayout::m_rotate_y,       StateValue::OFFSET },
        {"rotateZ",      &CommonItemLayout::m_rotate_z,       StateValue::OFFSET },
        {"scale",        &CommonItemLayout::m_scale,          StateValue::OFFSET },
        {"enterOffsetX", &CommonItemLayout::m_enter_offset_x, StateValue::OFFSET },
        {"enterOffsetY", &CommonItemLayout::m_enter_offset_y, StateValue::OFFSET },
        {"enterSizeX",   &CommonItemLayout::m_enter_size_x,   StateValue::OFFSET },
        {"enterSizeY",   &CommonItemLayout::m_enter_size_y,   StateValue::OFFSET },
        {"enterDepth",   &CommonItemLayout::m_enter_depth,    StateValue::OFFSET },
        {"enterRotateX", &CommonItemLayout::m_enter_rotate_x, StateValue::OFFSET },
        {"enterRotateY", &CommonItemLayout::m_enter_rotate_y, StateValue::OFFSET },
        {"enterRotateZ", &CommonItemLayout::m_enter_rotate_z, StateValue::OFFSET },
        {"enterScale",   &CommonItemLayout::m_enter_scale,    StateValue::OFFSET },
        {"enterOpacity", &CommonItemLayout::m_enter_opacity,  StateValue::OPACITY},
        {"enterDelay",   &CommonItemLayout::m_enter_delay,    StateValue::DELAY  },
        {"focusOffsetX", &CommonItemLayout::m_focus_offset_x, StateValue::OFFSET },
        {"focusOffsetY", &CommonItemLayout::m_focus_offset_y, StateValue::OFFSET },
        {"focusSizeX",   &CommonItemLayout::m_focus_size_x,   StateValue::OFFSET },
        {"focusSizeY",   &CommonItemLayout::m_focus_size_y,   StateValue::OFFSET },
        {"focusDepth",   &CommonItemLayout::m_focus_depth,    StateValue::OFFSET },
        {"focusRotateX", &CommonItemLayout::m_focus_rotate_x, StateValue::OFFSET },
        {"focusRotateY", &CommonItemLayout::m_focus_rotate_y, StateValue::OFFSET },
        {"focusRotateZ", &CommonItemLayout::m_focus_rotate_z, StateValue::OFFSET },
        {"focusScale",   &CommonItemLayout::m_focus_scale,    StateValue::OFFSET },
        {"exitOffsetX",  &CommonItemLayout::m_exit_offset_x,  StateValue::OFFSET },
        {"exitOffsetY",  &CommonItemLayout::m_exit_offset_y,  StateValue::OFFSET },
        {"exitSizeX",    &CommonItemLayout::m_exit_size_x,    StateValue::OFFSET },
        {"exitSizeY",    &CommonItemLayout::m_exit_size_y,    StateValue::OFFSET },
        {"exitDepth",    &CommonItemLayout::m_exit_depth,     StateValue::OFFSET },
        {"exitRotateX",  &CommonItemLayout::m_exit_rotate_x,  StateValue::OFFSET },
        {"exitRotateY",  &CommonItemLayout::m_exit_rotate_y,  StateValue::OFFSET },
        {"exitRotateZ",  &CommonItemLayout::m_exit_rotate_z,  StateValue::OFFSET },
        {"exitScale",    &CommonItemLayout::m_exit_scale,     StateValue::OFFSET },
        {"exitOpacity",  &CommonItemLayout::m_exit_opacity,   StateValue::OPACITY},
        {"exitDelay",    &CommonItemLayout::m_exit_delay,     StateValue::DELAY  },
    };

    for (const auto& property : states)
    {
        sequences.emplace_back(std::make_unique<GenericFloatingPointPropertySequence>(
            property.m_keyword,
            [property](const MenuFileParserState* state, const TokenPos& pos, const double value)
            {
                switch (property.m_value)
                {
                case StateValue::OFFSET:
                    break;

                case StateValue::OPACITY:
                    if (value < 0 || value > 1)
                        throw ParsingException(pos, std::format("Layout property \"{}\" must be from 0 to 1", property.m_keyword));
                    break;

                case StateValue::DELAY:
                    EnsureNonNegative(value, pos, property.m_keyword);
                    break;
                }

                SetOnce(CurrentItemLayout(state, pos).*property.m_member, value, pos, property.m_keyword);
            }));
    }

    for (const auto& set : PROPERTY_SETS)
        AddPropertySetSequences(sequences, set);

    return sequences;
}
