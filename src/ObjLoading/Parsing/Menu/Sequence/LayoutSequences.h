#pragma once

#include "Parsing/Menu/MenuFileParser.h"

#include <memory>
#include <vector>

namespace menu
{
    class LayoutSequences
    {
    public:
        // Throws unless the current menu declared a layoutVersion
        static void EnsureIsLayoutMenu(const MenuFileParserState* state, const TokenPos& pos);

        // Throws if the current menu declared a layoutVersion, naming what is not supported in layout menus
        static void EnsureIsNotLayoutMenu(const MenuFileParserState* state, const TokenPos& pos, const char* what);

        // Throws if the current item has compact layout properties without compactBelow
        static void EnsureCompactPropertiesHaveThreshold(const MenuFileParserState* state, const TokenPos& pos);

        // Throws if the current item exits before it enters
        static void EnsureExitFollowsEnter(const MenuFileParserState* state, const TokenPos& pos);

        // Sets the offsets and the fixed size of the current item from its rect, which in a layout menu is shorthand for them
        static void SetRect(const MenuFileParserState* state, const TokenPos& pos, double x, double y, double w, double h);

        static std::vector<std::unique_ptr<MenuFileParser::sequence_t>> CreateMenuSequences();
        static std::vector<std::unique_ptr<MenuFileParser::sequence_t>> CreateItemSequences();
    };
} // namespace menu
