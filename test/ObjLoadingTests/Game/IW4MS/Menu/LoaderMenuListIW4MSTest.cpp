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

        [[nodiscard]] bool HasRawFile(const std::string& name) const
        {
            return m_zone.m_pools.GetAsset(ASSET_TYPE_RAWFILE, name) != nullptr;
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

    TEST_CASE("MenuParsingIW4MSIT: Rejects invalid layout menus", "[parsing][converting][menu][layout][it]")
    {
        const auto [description, menu] = GENERATE(table<const char*, const char*>({
            {"layout property without layoutVersion", R"(menuDef { name "m" itemDef { anchor left top } })"                              },
            {"layoutSpace without layoutVersion",     R"(menuDef { name "m" layoutSpace safe })"                                         },
            {"unsupported layoutVersion",             R"(menuDef { name "m" layoutVersion 2 })"                                          },
            {"layoutVersion after itemDef",           R"(menuDef { name "m" itemDef { } layoutVersion 1 })"                              },
            {"rect alignment",                        R"(menuDef { name "m" layoutVersion 1 itemDef { rect 0 0 10 10 1 1 } })"           },
            {"origin",                                R"(menuDef { name "m" layoutVersion 1 itemDef { origin 4 4 } })"                   },
            {"rect expression",                       R"(menuDef { name "m" layoutVersion 1 itemDef { exp rect X(4) } })"                },
            {"repeated property",                     R"(menuDef { name "m" layoutVersion 1 itemDef { offsetX 1 offsetX 2 } })"          },
            {"width and widthPercent",                R"(menuDef { name "m" layoutVersion 1 itemDef { width 10 widthPercent 0.5 } })"    },
            {"rect and offsetX",                      R"(menuDef { name "m" layoutVersion 1 itemDef { offsetX 1 rect 0 0 10 10 } })"     },
            {"stretch anchor and width",              R"(menuDef { name "m" layoutVersion 1 itemDef { width 10 anchor stretch top } })"  },
            {"width and stretch anchor",              R"(menuDef { name "m" layoutVersion 1 itemDef { anchor left stretch height 10 } })"},
            {"compact property without compactBelow", R"(menuDef { name "m" layoutVersion 1 itemDef { compactWidth 10 } })"              },
            {"invalid horizontal anchor",             R"(menuDef { name "m" layoutVersion 1 itemDef { anchor top top } })"               },
            {"invalid layout space",                  R"(menuDef { name "m" layoutVersion 1 layoutSpace wide })"                         },
            {"invalid flow",                          R"(menuDef { name "m" layoutVersion 1 itemDef { flow diagonal } })"                },
            {"invalid fit",                           R"(menuDef { name "m" layoutVersion 1 itemDef { fit fill } })"                     },
            {"three padding values",                  R"(menuDef { name "m" layoutVersion 1 itemDef { padding 1 2 3 } })"                },
            {"one aspect ratio value",                R"(menuDef { name "m" layoutVersion 1 itemDef { aspectRatio 16 } })"               },
            {"zero aspect ratio value",               R"(menuDef { name "m" layoutVersion 1 itemDef { aspectRatio 16 0 } })"             },
            {"negative width",                        R"(menuDef { name "m" layoutVersion 1 itemDef { width -1 } })"                     },
            {"wrap value",                            R"(menuDef { name "m" layoutVersion 1 itemDef { wrap 2 } })"                       },
            {"zero compactBelow",                     R"(menuDef { name "m" layoutVersion 1 itemDef { compactBelow 0 } })"               },
            {"repeated clipChildren",                 R"(menuDef { name "m" layoutVersion 1 itemDef { clipChildren clipChildren } })"    },
        }));

        INFO(description);

        MenuParsingItHelper helper;
        helper.AddFile(std::format("{{ {} }}", menu));

        const auto result = helper.RunIntegrationTest();
        REQUIRE(result.HasFailed());
        REQUIRE(!helper.HasRawFile("layout/m"));
    }
} // namespace test::game::iw4ms::menu::parsing::it
