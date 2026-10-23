#include "Game/IW4MS/Menu/LoaderMenuListIW4MS.h"

#include "Game/IW4MS/GameIW4MS.h"
#include "SearchPath/MockSearchPath.h"
#include "Utils/MemoryManager.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <format>
#include <string>

using namespace IW4MS;
using namespace std::literals;

namespace test::game::iw4ms::menu::parsing::it
{
    class MenuParsingItHelper
    {
        static constexpr const char* DEFAULT_ASSET_NAME = "test.txt";

        Zone m_zone;
        MockSearchPath m_search_path;
        std::unique_ptr<IAssetCreator> m_asset_creator;
        AssetCreatorCollection m_creator_collection;
        IgnoredAssetLookup m_ignored_asset_lookup;
        AssetCreationContext m_context;

    public:
        MenuParsingItHelper()
            : m_zone("MockZone", 0, GameId::IW4MS, GamePlatform::PC),
              m_creator_collection(m_zone),
              m_ignored_asset_lookup(),
              m_context(m_zone, &m_creator_collection, &m_ignored_asset_lookup)
        {
            m_asset_creator = ::menu::CreateMenuListLoaderIW4MS(m_zone.Memory(), m_search_path);
        }

        void AddFile(std::string data)
        {
            m_search_path.AddFileData(DEFAULT_ASSET_NAME, std::move(data));
        }

        AssetCreationResult RunIntegrationTest()
        {
            return m_asset_creator->CreateAsset(DEFAULT_ASSET_NAME, m_context);
        }

        [[nodiscard]] const menuDef_t* GetMenu(const std::string& name) const
        {
            const auto addedAsset = m_zone.m_pools.GetAsset(ASSET_TYPE_MENU, name);
            REQUIRE(addedAsset);

            return static_cast<const menuDef_t*>(addedAsset->m_ptr);
        }

        [[nodiscard]] bool HasRawFile(const std::string& name) const
        {
            return m_zone.m_pools.GetAsset(ASSET_TYPE_RAWFILE, name) != nullptr;
        }

        std::string GetRawFileText(const std::string& name)
        {
            const auto addedAsset = m_zone.m_pools.GetAsset(ASSET_TYPE_RAWFILE, name);
            REQUIRE(addedAsset);

            const auto* rawFile = static_cast<RawFile*>(addedAsset->m_ptr);
            REQUIRE(rawFile->compressedLen == 0);
            REQUIRE(rawFile->data.buffer);

            return std::string(rawFile->data.buffer, static_cast<std::size_t>(rawFile->len));
        }
    };

    TEST_CASE("MenuParsingIW4MSIT: Writes no layout description for a legacy menu", "[parsing][converting][menu][layout][it]")
    {
        MenuParsingItHelper helper;

        helper.AddFile(R"testmenu(
{
	menuDef
	{
		name "Hello"
		itemDef
		{
			name "Item"
			rect 10 20 30 40 1 1
		}
	}
}
)testmenu");

        const auto result = helper.RunIntegrationTest();
        REQUIRE(result.HasBeenSuccessful());
        REQUIRE(!helper.HasRawFile("layout/Hello"));
    }

    TEST_CASE("MenuParsingIW4MSIT: Writes the layout description of a layout menu", "[parsing][converting][menu][layout][it]")
    {
        MenuParsingItHelper helper;

        helper.AddFile(R"testmenu(
{
	menuDef
	{
		name "main_menu"
		layoutVersion 1
		layoutSpace protected

		itemDef
		{
			name "backdrop"
			layoutSpace full
			anchor stretch stretch
			fit cover
		}

		itemDef
		{
			name "panel"
			layoutSpace safe
			anchor center center
			width 1120
			height 760
			padding 48
			flow vertical
			flowAlign end
			itemSpacing 24
			wrap
			clipChildren
			fitText
			compactBelow 1800
			compactWidthPercent 1.0
			compactPadding 24 12
			compactWrap 0
			compactFlowAlign center
		}

		itemDef
		{
			name "title"
			parent "panel"
			rect 0 -8 400 80
			maxWidth 960
		}

		itemDef
		{
			name "logo"
			parent "backdrop"
			anchor right bottom
			offsetX -48
			width 640
			aspectRatio 16 9
		}

		itemDef
		{
		}
	}
}
)testmenu");

        const auto result = helper.RunIntegrationTest();
        REQUIRE(result.HasBeenSuccessful());

        REQUIRE(helper.GetRawFileText("layout/main_menu") == R"(layoutVersion 1
menu "main_menu" 5
layoutSpace protected

item 0 "backdrop"
layoutSpace full
anchor stretch stretch
fit cover

item 1 "panel"
layoutSpace safe
anchor center center
width 1120
height 760
padding 48 48 48 48
flow vertical
flowAlign end
itemSpacing 24
wrap 1
clipChildren
fitText
compactBelow 1800
compactWidthPercent 1
compactPadding 24 12 24 12
compactFlowAlign center
compactWrap 0

item 2 "title"
parent "panel"
offsetX 0
offsetY -8
width 400
height 80
maxWidth 960

item 3 "logo"
parent "backdrop"
anchor right bottom
offsetX -48
width 640
aspectRatio 16 9

item 4 ""
)"s);
    }

    TEST_CASE("MenuParsingIW4MSIT: Escapes names in the layout description", "[parsing][converting][menu][layout][it]")
    {
        MenuParsingItHelper helper;

        helper.AddFile(R"testmenu(
{
	menuDef
	{
		name "quote"
		layoutVersion 1
		itemDef
		{
			name "a\"b\\c"
		}
	}
}
)testmenu");

        const auto result = helper.RunIntegrationTest();
        REQUIRE(result.HasBeenSuccessful());

        REQUIRE(helper.GetRawFileText("layout/quote") == R"(layoutVersion 1
menu "quote" 1

item 0 "a\"b\\c"
)"s);
    }

    TEST_CASE("MenuParsingIW4MSIT: Writes the motion of a layout menu", "[parsing][converting][menu][layout][it]")
    {
        MenuParsingItHelper helper;

        helper.AddFile(R"testmenu(
{
	menuDef
	{
		name "motion"
		layoutVersion 1
		motion snappy
		itemDef
		{
			name "panel"
			motion response 2.5 0.5 (0.5 + 0.5)
			enterOffsetY 40
			enterOpacity 0
		}
		itemDef
		{
			name "button"
			motion spring 120 22 RK4
			focusOffsetX 8
			focusOffsetY -4
			focusSizeY (50 + 24)
		}
		itemDef
		{
			name "label"
			motion none
			enterOffsetX -10
		}
		itemDef
		{
			name "plain"
			motion springy
		}
	}
}
)testmenu");

        const auto result = helper.RunIntegrationTest();
        REQUIRE(result.HasBeenSuccessful());

        REQUIRE(helper.GetRawFileText("layout/motion") == R"(layoutVersion 1
menu "motion" 4
motion snappy

item 0 "panel"
motion response 2.5 0.5 1
enterOffsetY 40
enterOpacity 0

item 1 "button"
motion spring 120 22 rk4
focusOffsetX 8
focusOffsetY -4
focusSizeY 74

item 2 "label"
motion none
enterOffsetX -10

item 3 "plain"
motion springy
)"s);
    }

    TEST_CASE("MenuParsingIW4MSIT: Writes the timeline of a layout menu", "[parsing][converting][menu][layout][it]")
    {
        MenuParsingItHelper helper;

        helper.AddFile(R"testmenu(
{
	menuDef
	{
		name "timeline"
		layoutVersion 1
		itemDef
		{
			name "title"
			exitDelay 6.5
			enterOpacity 0
			enterDelay 0.75
			exitOffsetY -40
			exitOpacity 0
		}
		itemDef
		{
			name "rule"
			exitOffsetX 1920
			exitDelay 0
			enterSizeX -400
			exitSizeY 8
		}
	}
}
)testmenu");

        const auto result = helper.RunIntegrationTest();
        REQUIRE(result.HasBeenSuccessful());

        REQUIRE(helper.GetRawFileText("layout/timeline") == R"(layoutVersion 1
menu "timeline" 2

item 0 "title"
enterOpacity 0
enterDelay 0.75
exitOffsetY -40
exitOpacity 0
exitDelay 6.5

item 1 "rule"
enterSizeX -400
exitOffsetX 1920
exitSizeY 8
exitDelay 0
)"s);
    }

    TEST_CASE("MenuParsingIW4MSIT: Writes the pose of a layout menu", "[parsing][converting][menu][layout][it]")
    {
        MenuParsingItHelper helper;

        helper.AddFile(R"testmenu(
{
	menuDef
	{
		name "pose"
		layoutVersion 1
		perspective 1200
		itemDef
		{
			name "card"
			exitScale 2
			rotateY -20.5
			depth -50
			enterRotateY (45 * 2)
			enterScale 0
			enterDepth 400
			focusRotateX -5
			focusDepth 30
			focusScale 1.1
			exitRotateZ -15
			exitDepth -800
		}
		itemDef
		{
			name "tilt"
			rotateX 10
			rotateZ 45
			scale 1.5
			enterRotateX 90
			enterRotateZ 15
			focusRotateY 12
			focusRotateZ 2
			exitRotateX 0
			exitRotateY 90
		}
	}
}
)testmenu");

        const auto result = helper.RunIntegrationTest();
        REQUIRE(result.HasBeenSuccessful());

        REQUIRE(helper.GetRawFileText("layout/pose") == R"(layoutVersion 1
menu "pose" 2
perspective 1200

item 0 "card"
depth -50
rotateY -20.5
enterDepth 400
enterRotateY 90
enterScale 0
focusDepth 30
focusRotateX -5
focusScale 1.1
exitDepth -800
exitRotateZ -15
exitScale 2

item 1 "tilt"
rotateX 10
rotateZ 45
scale 1.5
enterRotateX 90
enterRotateZ 15
focusRotateY 12
focusRotateZ 2
exitRotateX 0
exitRotateY 90
)"s);
    }

    TEST_CASE("MenuParsingIW4MSIT: Writes the lighting and surfaces of a layout menu", "[parsing][converting][menu][layout][it]")
    {
        MenuParsingItHelper helper;

        helper.AddFile(R"testmenu(
{
	menuDef
	{
		name "solid"
		layoutVersion 1
		perspective 1200
		light -1 -2 3
		itemDef
		{
			name "face"
			shade 0.25
			backfaceHidden
		}
		itemDef
		{
			name "plain"
			shade 1
		}
	}
}
)testmenu");

        const auto result = helper.RunIntegrationTest();
        REQUIRE(result.HasBeenSuccessful());

        REQUIRE(helper.GetRawFileText("layout/solid") == R"(layoutVersion 1
menu "solid" 2
perspective 1200
light -1 -2 3

item 0 "face"
backfaceHidden
shade 0.25

item 1 "plain"
shade 1
)"s);
    }

    TEST_CASE("MenuParsingIW4MSIT: Writes the corner radius of layout items", "[parsing][converting][menu][layout][it]")
    {
        MenuParsingItHelper helper;

        helper.AddFile(R"testmenu(
{
	menuDef
	{
		name "rounded"
		layoutVersion 1
		itemDef
		{
			name "card"
			radius 12.5
		}
		itemDef
		{
			name "square"
		}
	}
}
)testmenu");

        const auto result = helper.RunIntegrationTest();
        REQUIRE(result.HasBeenSuccessful());

        REQUIRE(helper.GetRawFileText("layout/rounded") == R"(layoutVersion 1
menu "rounded" 2

item 0 "card"
radius 12.5

item 1 "square"
)"s);
    }

    TEST_CASE("MenuParsingIW4MSIT: Keeps width and height expressions of layout items", "[parsing][converting][menu][layout][it]")
    {
        MenuParsingItHelper helper;

        helper.AddFile(R"testmenu(
{
	menuDef
	{
		name "bars"
		layoutVersion 1
		itemDef
		{
			name "fill"
			width 100
			height 2
			exp rect W(dvarfloat("progress") * 100)
			exp rect H(dvarint("thick"))
		}
	}
}
)testmenu");

        const auto result = helper.RunIntegrationTest();
        REQUIRE(result.HasBeenSuccessful());

        const auto* menu = helper.GetMenu("bars");
        REQUIRE(menu->itemCount == 1);

        const auto* item = menu->items[0];
        REQUIRE(item->floatExpressionCount == 2);
        REQUIRE(item->floatExpressions[0].target == ITEM_FLOATEXP_TGT_RECT_W);
        REQUIRE(item->floatExpressions[1].target == ITEM_FLOATEXP_TGT_RECT_H);

        REQUIRE(helper.GetRawFileText("layout/bars") == R"(layoutVersion 1
menu "bars" 1

item 0 "fill"
width 100
height 2
)"s);
    }

    TEST_CASE("MenuParsingIW4MSIT: Keeps static width and height expressions of layout items", "[parsing][converting][menu][layout][it]")
    {
        MenuParsingItHelper helper;

        helper.AddFile(R"testmenu(
{
	menuDef
	{
		name "mirrored"
		layoutVersion 1
		itemDef
		{
			name "symbol"
			width 64
			height 32
			exp rect W(-64)
			exp rect H(32)
		}
	}
}
)testmenu");

        const auto result = helper.RunIntegrationTest();
        REQUIRE(result.HasBeenSuccessful());

        const auto* menu = helper.GetMenu("mirrored");
        REQUIRE(menu->itemCount == 1);

        const auto* item = menu->items[0];
        REQUIRE(item->floatExpressionCount == 2);
        REQUIRE(item->floatExpressions[0].target == ITEM_FLOATEXP_TGT_RECT_W);
        REQUIRE(item->floatExpressions[1].target == ITEM_FLOATEXP_TGT_RECT_H);
    }

    TEST_CASE("MenuParsingIW4MSIT: Writes the map a layout item stands for", "[parsing][converting][menu][layout][it]")
    {
        MenuParsingItHelper helper;

        helper.AddFile(R"testmenu(
{
	menuDef
	{
		name "showcase"
		layoutVersion 1
		itemDef
		{
			name "world"
			scene "mp_rust" 120 -340.5 64
		}
		itemDef
		{
			name "close"
			scene "mp_rust" 0 0 0 12.5
		}
	}
}
)testmenu");

        const auto result = helper.RunIntegrationTest();
        REQUIRE(result.HasBeenSuccessful());

        REQUIRE(helper.GetRawFileText("layout/showcase") == R"(layoutVersion 1
menu "showcase" 2

item 0 "world"
scene "mp_rust" 120 -340.5 64

item 1 "close"
scene "mp_rust" 0 0 0 12.5
)"s);
    }

    TEST_CASE("MenuParsingIW4MSIT: Writes the subject and tethers of layout items", "[parsing][converting][menu][layout][it]")
    {
        MenuParsingItHelper helper;

        helper.AddFile(R"testmenu(
{
	menuDef
	{
		name "gunsmith"
		layoutVersion 1
		itemDef
		{
			name "world"
			scene "mp_rust" 0 0 0 8
			subject "weapon"
		}
		itemDef
		{
			name "optic"
			tether "tag_reflex tag_sight_on"
		}
	}
}
)testmenu");

        const auto result = helper.RunIntegrationTest();
        REQUIRE(result.HasBeenSuccessful());

        REQUIRE(helper.GetRawFileText("layout/gunsmith") == R"(layoutVersion 1
menu "gunsmith" 2

item 0 "world"
scene "mp_rust" 0 0 0 8
subject "weapon"

item 1 "optic"
tether "tag_reflex" "tag_sight_on"
)"s);
    }

    TEST_CASE("MenuParsingIW4MSIT: Writes the scrolling of layout items", "[parsing][converting][menu][layout][it]")
    {
        MenuParsingItHelper helper;

        helper.AddFile(R"testmenu(
{
	menuDef
	{
		name "cards"
		layoutVersion 1
		itemDef
		{
			name "grid"
			flow horizontal
			wrap 1
			scroll vertical
			virtual "ui_first"
		}
		itemDef
		{
			name "thumb"
			scrollThumb "grid"
		}
		itemDef
		{
			name "strip"
			flow horizontal
			scroll horizontal
		}
	}
}
)testmenu");

        const auto result = helper.RunIntegrationTest();
        REQUIRE(result.HasBeenSuccessful());

        REQUIRE(helper.GetRawFileText("layout/cards") == R"(layoutVersion 1
menu "cards" 3

item 0 "grid"
flow horizontal
wrap 1
scroll vertical
virtual "ui_first"

item 1 "thumb"
scrollThumb "grid"

item 2 "strip"
flow horizontal
scroll horizontal
)"s);
    }

    TEST_CASE("MenuParsingIW4MSIT: Rejects invalid layout menus", "[parsing][converting][menu][layout][it]")
    {
        const auto [description, menu] = GENERATE(table<const char*, const char*>({
            {"layout property without layoutVersion", R"(menuDef { name "m" itemDef { anchor left top } })"                                },
            {"layoutSpace without layoutVersion",     R"(menuDef { name "m" layoutSpace safe })"                                           },
            {"unsupported layoutVersion",             R"(menuDef { name "m" layoutVersion 2 })"                                            },
            {"layoutVersion after itemDef",           R"(menuDef { name "m" itemDef { } layoutVersion 1 })"                                },
            {"rect alignment",                        R"(menuDef { name "m" layoutVersion 1 itemDef { rect 0 0 10 10 1 1 } })"             },
            {"origin",                                R"(menuDef { name "m" layoutVersion 1 itemDef { origin 4 4 } })"                     },
            {"rect x expression",                     R"(menuDef { name "m" layoutVersion 1 itemDef { exp rect X(4) } })"                  },
            {"rect y expression",                     R"(menuDef { name "m" layoutVersion 1 itemDef { exp rect Y(4) } })"                  },
            {"repeated property",                     R"(menuDef { name "m" layoutVersion 1 itemDef { offsetX 1 offsetX 2 } })"            },
            {"width and widthPercent",                R"(menuDef { name "m" layoutVersion 1 itemDef { width 10 widthPercent 0.5 } })"      },
            {"rect and offsetX",                      R"(menuDef { name "m" layoutVersion 1 itemDef { offsetX 1 rect 0 0 10 10 } })"       },
            {"stretch anchor and width",              R"(menuDef { name "m" layoutVersion 1 itemDef { width 10 anchor stretch top } })"    },
            {"width and stretch anchor",              R"(menuDef { name "m" layoutVersion 1 itemDef { anchor left stretch height 10 } })"  },
            {"compact property without compactBelow", R"(menuDef { name "m" layoutVersion 1 itemDef { compactWidth 10 } })"                },
            {"invalid horizontal anchor",             R"(menuDef { name "m" layoutVersion 1 itemDef { anchor top top } })"                 },
            {"invalid layout space",                  R"(menuDef { name "m" layoutVersion 1 layoutSpace wide })"                           },
            {"invalid flow",                          R"(menuDef { name "m" layoutVersion 1 itemDef { flow diagonal } })"                  },
            {"invalid fit",                           R"(menuDef { name "m" layoutVersion 1 itemDef { fit fill } })"                       },
            {"three padding values",                  R"(menuDef { name "m" layoutVersion 1 itemDef { padding 1 2 3 } })"                  },
            {"one aspect ratio value",                R"(menuDef { name "m" layoutVersion 1 itemDef { aspectRatio 16 } })"                 },
            {"zero aspect ratio value",               R"(menuDef { name "m" layoutVersion 1 itemDef { aspectRatio 16 0 } })"               },
            {"negative width",                        R"(menuDef { name "m" layoutVersion 1 itemDef { width -1 } })"                       },
            {"wrap value",                            R"(menuDef { name "m" layoutVersion 1 itemDef { wrap 2 } })"                         },
            {"zero compactBelow",                     R"(menuDef { name "m" layoutVersion 1 itemDef { compactBelow 0 } })"                 },
            {"repeated clipChildren",                 R"(menuDef { name "m" layoutVersion 1 itemDef { clipChildren clipChildren } })"      },
            {"repeated fitText",                      R"(menuDef { name "m" layoutVersion 1 itemDef { fitText fitText } })"                },
            {"menu motion without layoutVersion",     R"(menuDef { name "m" motion snappy })"                                              },
            {"item motion without layoutVersion",     R"(menuDef { name "m" itemDef { motion snappy } })"                                  },
            {"focus offset without layoutVersion",    R"(menuDef { name "m" itemDef { focusOffsetX 4 } })"                                 },
            {"repeated menu motion",                  R"(menuDef { name "m" layoutVersion 1 motion snappy motion smooth })"                },
            {"repeated item motion",                  R"(menuDef { name "m" layoutVersion 1 itemDef { motion none motion smooth } })"      },
            {"invalid motion",                        R"(menuDef { name "m" layoutVersion 1 itemDef { motion bouncy } })"                  },
            {"preset motion with values",             R"(menuDef { name "m" layoutVersion 1 itemDef { motion smooth 2 1 0 } })"            },
            {"response with two values",              R"(menuDef { name "m" layoutVersion 1 itemDef { motion response 2 1 } })"            },
            {"response with an integrator",           R"(menuDef { name "m" layoutVersion 1 itemDef { motion response 2 1 rk4 } })"        },
            {"spring without an integrator",          R"(menuDef { name "m" layoutVersion 1 itemDef { motion spring 100 20 } })"           },
            {"spring with three values",              R"(menuDef { name "m" layoutVersion 1 itemDef { motion spring 100 20 0 } })"         },
            {"invalid integrator",                    R"(menuDef { name "m" layoutVersion 1 itemDef { motion spring 100 20 verlet } })"    },
            {"zero response frequency",               R"(menuDef { name "m" layoutVersion 1 itemDef { motion response 0 1 0 } })"          },
            {"negative response damping",             R"(menuDef { name "m" layoutVersion 1 itemDef { motion response 2 -1 0 } })"         },
            {"zero spring stiffness",                 R"(menuDef { name "m" layoutVersion 1 itemDef { motion spring 0 20 rk4 } })"         },
            {"negative spring damping",               R"(menuDef { name "m" layoutVersion 1 itemDef { motion spring 100 -1 rk4 } })"       },
            {"enterOpacity above 1",                  R"(menuDef { name "m" layoutVersion 1 itemDef { enterOpacity 1.5 } })"               },
            {"negative enterOpacity",                 R"(menuDef { name "m" layoutVersion 1 itemDef { enterOpacity -0.5 } })"              },
            {"repeated focusOffsetY",                 R"(menuDef { name "m" layoutVersion 1 itemDef { focusOffsetY 1 focusOffsetY 2 } })"  },
            {"exitOpacity above 1",                   R"(menuDef { name "m" layoutVersion 1 itemDef { exitOpacity 2 } })"                  },
            {"negative enterDelay",                   R"(menuDef { name "m" layoutVersion 1 itemDef { enterDelay -1 } })"                  },
            {"exitDelay before enterDelay",           R"(menuDef { name "m" layoutVersion 1 itemDef { exitDelay 1 enterDelay 2 } })"       },
            {"exitDelay without layoutVersion",       R"(menuDef { name "m" itemDef { exitDelay 1 } })"                                    },
            {"perspective without layoutVersion",     R"(menuDef { name "m" perspective 800 })"                                            },
            {"zero perspective",                      R"(menuDef { name "m" layoutVersion 1 perspective 0 })"                              },
            {"repeated perspective",                  R"(menuDef { name "m" layoutVersion 1 perspective 800 perspective 900 })"            },
            {"light without layoutVersion",           R"(menuDef { name "m" light 0 0 1 })"                                                },
            {"light without a direction",             R"(menuDef { name "m" layoutVersion 1 light 0 0 0 })"                                },
            {"light with two values",                 R"(menuDef { name "m" layoutVersion 1 light 0 1 })"                                  },
            {"repeated light",                        R"(menuDef { name "m" layoutVersion 1 light 0 0 1 light 0 1 0 })"                    },
            {"shade above 1",                         R"(menuDef { name "m" layoutVersion 1 itemDef { shade 1.5 } })"                      },
            {"repeated shade",                        R"(menuDef { name "m" layoutVersion 1 itemDef { shade 0.5 shade 0.5 } })"            },
            {"repeated backfaceHidden",               R"(menuDef { name "m" layoutVersion 1 itemDef { backfaceHidden backfaceHidden } })"  },
            {"negative radius",                       R"(menuDef { name "m" layoutVersion 1 itemDef { radius -1 } })"                      },
            {"repeated radius",                       R"(menuDef { name "m" layoutVersion 1 itemDef { radius 4 radius 4 } })"              },
            {"radius without layoutVersion",          R"(menuDef { name "m" itemDef { radius 4 } })"                                       },
            {"scene with too few values",             R"(menuDef { name "m" layoutVersion 1 itemDef { scene "mp_rust" 1 2 } })"            },
            {"scene with too many values",            R"(menuDef { name "m" layoutVersion 1 itemDef { scene "mp_rust" 1 2 3 4 5 } })"      },
            {"scene with zero size",                  R"(menuDef { name "m" layoutVersion 1 itemDef { scene "mp_rust" 1 2 3 0 } })"        },
            {"scene without map",                     R"(menuDef { name "m" layoutVersion 1 itemDef { scene "" 1 2 3 } })"                 },
            {"repeated scene",                        R"(menuDef { name "m" layoutVersion 1 itemDef { scene "a" 0 0 0 scene "a" 0 0 0 } })"},
            {"scene without layoutVersion",           R"(menuDef { name "m" itemDef { scene "mp_rust" 0 0 0 } })"                          },
            {"empty subject",                         R"(menuDef { name "m" layoutVersion 1 itemDef { subject "" } })"                     },
            {"repeated subject",                      R"(menuDef { name "m" layoutVersion 1 itemDef { subject "a" subject "a" } })"        },
            {"subject without layoutVersion",         R"(menuDef { name "m" itemDef { subject "weapon" } })"                               },
            {"empty tether",                          R"(menuDef { name "m" layoutVersion 1 itemDef { tether " " } })"                     },
            {"repeated tether",                       R"(menuDef { name "m" layoutVersion 1 itemDef { tether "a" tether "b" } })"          },
            {"tether without layoutVersion",          R"(menuDef { name "m" itemDef { tether "tag_flash" } })"                             },
            {"rotateY without layoutVersion",         R"(menuDef { name "m" itemDef { rotateY 90 } })"                                     },
            {"repeated enterScale",                   R"(menuDef { name "m" layoutVersion 1 itemDef { enterScale 0 enterScale 1 } })"      },
            {"invalid scroll",                        R"(menuDef { name "m" layoutVersion 1 itemDef { scroll diagonal } })"                },
            {"repeated scroll",                       R"(menuDef { name "m" layoutVersion 1 itemDef { scroll vertical scroll vertical } })"},
            {"scroll without layoutVersion",          R"(menuDef { name "m" itemDef { scroll vertical } })"                                },
            {"empty scrollThumb",                     R"(menuDef { name "m" layoutVersion 1 itemDef { scrollThumb "" } })"                 },
            {"repeated scrollThumb",                  R"(menuDef { name "m" layoutVersion 1 itemDef { scrollThumb "a" scrollThumb "a" } })"},
            {"empty virtual",                         R"(menuDef { name "m" layoutVersion 1 itemDef { virtual "" } })"                     },
            {"repeated virtual",                      R"(menuDef { name "m" layoutVersion 1 itemDef { virtual "a" virtual "a" } })"        },
        }));

        INFO(description);

        MenuParsingItHelper helper;
        helper.AddFile(std::format("{{ {} }}", menu));

        const auto result = helper.RunIntegrationTest();
        REQUIRE(result.HasFailed());
        REQUIRE(!helper.HasRawFile("layout/m"));
    }
} // namespace test::game::iw4ms::menu::parsing::it
