#pragma once

#include "Parsing/Menu/Domain/CommonMenuDef.h"

#include <string>

namespace menu
{
    // Creates the layout description of a menu that declares a layoutVersion, which is written next to the compiled menu as the rawfile
    // returned by GetLayoutDescriptionName.
    //
    // The description is line based, one statement per line:
    //
    //   layoutVersion 1
    //   menu "<name>" <item count>
    //   [layoutSpace <space>]
    //   [motion <motion>]
    //   item <index> "<name>"
    //   <item property> <value>...
    //   ...
    //
    // Every item of the menu is listed in the order of the compiled menu's items, so the runtime finds an item by its index and verifies the
    // pairing by its name. Names are double-quoted with \" and \\ escapes. The item properties are the layout keywords of the menu source
    // with their values, compact ones included, padding always with its four values, followed by the motion and the enter and focus
    // states. A motion is written as in the source, with the integrator of a spring in lowercase.
    std::string CreateLayoutDescription(const CommonMenuDef& menu);

    std::string GetLayoutDescriptionName(const std::string& menuName);
} // namespace menu
