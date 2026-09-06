#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/texture_manager.hpp"
#include "../src/font_manager.hpp"
#include "../src/gui_manager.hpp"
#include "../src/constants.hpp"

TEST_CASE("TextureManager functionality", "[texture_manager]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    TextureManager& texManager = manager.getTextureManager();

    SECTION("Load texture from file") {
        auto tex = texManager.loadTexture("assets/button1.png");
        REQUIRE(tex != nullptr);
        
        float fw, fh;
        SDL_GetTextureSize(tex.get(), &fw, &fh);
        int w = static_cast<int>(fw);
        int h = static_cast<int>(fh);
        REQUIRE(w > 0);
        REQUIRE(h > 0);
    }

    SECTION("Texture is cached after first load") {
        auto tex1 = texManager.loadTexture("assets/button1.png");
        auto tex2 = texManager.loadTexture("assets/button1.png");
        REQUIRE(tex1 == tex2);
        REQUIRE(tex1.get() == tex2.get());
    }

    SECTION("Loading non-existent texture returns nullptr") {
        auto tex = texManager.loadTexture("nonexistent/path.png");
        REQUIRE(tex == nullptr);
    }

    SECTION("hasTexture checks for texture existence") {
        REQUIRE_FALSE(texManager.hasTexture("test_key"));
        
        auto tex = helper.makeStubTexture(100, 100);
        texManager.addTexture("test_key", tex);
        
        REQUIRE(texManager.hasTexture("test_key"));
    }

    SECTION("getTexture retrieves added texture") {
        auto tex = helper.makeStubTexture(50, 50);
        texManager.addTexture("my_texture", tex);
        
        auto retrieved = texManager.getTexture("my_texture");
        REQUIRE(retrieved == tex);
    }

    SECTION("getTexture returns nullptr for unknown key") {
        auto retrieved = texManager.getTexture("unknown_key");
        REQUIRE(retrieved == nullptr);
    }

    SECTION("addTexture with SharedTexture") {
        auto tex = helper.makeStubTexture(100, 100);
        auto added = texManager.addTexture("shared_tex", tex);
        
        REQUIRE(added != nullptr);
        REQUIRE(added == tex);
        REQUIRE(texManager.hasTexture("shared_tex"));
    }

    SECTION("addTexture prevents duplicate keys") {
        auto tex1 = helper.makeStubTexture(100, 100);
        auto tex2 = helper.makeStubTexture(200, 200);
        
        auto added1 = texManager.addTexture("dup_key", tex1);
        auto added2 = texManager.addTexture("dup_key", tex2);
        
        REQUIRE(added1 == added2);
        REQUIRE(added1.get() == tex1.get());
    }

    SECTION("queryTexture returns dimensions") {
        int w, h;
        bool success = texManager.queryTexture("assets/button1.png", w, h);
        
        REQUIRE(success);
        REQUIRE(w > 0);
        REQUIRE(h > 0);
    }

    SECTION("queryTexture fails for non-existent file") {
        int w, h;
        bool success = texManager.queryTexture("nonexistent.png", w, h);
        
        REQUIRE_FALSE(success);
    }

    SECTION("queryTexture uses cached texture") {
        int w1, h1;
        texManager.queryTexture("assets/button1.png", w1, h1);
        
        REQUIRE(texManager.hasTexture("assets/button1.png"));
        
        int w2, h2;
        texManager.queryTexture("assets/button1.png", w2, h2);
        
        REQUIRE(w1 == w2);
        REQUIRE(h1 == h2);
    }
}

TEST_CASE("TextureManager TextShaper", "[texture_manager][text]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    TextureManager& texManager = manager.getTextureManager();
    FontManager& fontManager = manager.getFontManager();

    auto font16 = fontManager.loadFont(constants::kDefaultFontPath, 16);
    auto font24 = fontManager.loadFont(constants::kDefaultFontPath, 24);
    REQUIRE(font16);
    REQUIRE(font24);
    const SDL_Color black{0, 0, 0, 255};
    const SDL_Color red{255, 0, 0, 255};

    SECTION("identical text shares one texture") {
        const size_t before = texManager.getTextCacheSize();
        auto t1 = texManager.createTextureFromText("shared", font16, black);
        auto t2 = texManager.createTextureFromText("shared", font16, black);
        REQUIRE(t1);
        REQUIRE(t2.get() == t1.get());
        REQUIRE(texManager.getTextCacheSize() == before + 1);
    }

    SECTION("color is part of the key") {
        auto t1 = texManager.createTextureFromText("colorkey", font16, black);
        auto t2 = texManager.createTextureFromText("colorkey", font16, red);
        REQUIRE(t1);
        REQUIRE(t2);
        REQUIRE(t2.get() != t1.get());
    }

    SECTION("font is part of the key") {
        auto t1 = texManager.createTextureFromText("fontkey", font16, black);
        auto t2 = texManager.createTextureFromText("fontkey", font24, black);
        REQUIRE(t1);
        REQUIRE(t2);
        REQUIRE(t2.get() != t1.get());
    }

    SECTION("different texts do not collide") {
        auto t1 = texManager.createTextureFromText("alpha", font16, black);
        auto t2 = texManager.createTextureFromText("beta", font16, black);
        REQUIRE(t1);
        REQUIRE(t2);
        REQUIRE(t2.get() != t1.get());
    }

    SECTION("grid-scale sharing: N identical cells, 1 entry") {
        const size_t before = texManager.getTextCacheSize();
        for (int i = 0; i < 100; ++i) {
            auto t = texManager.createTextureFromText("cell", font16, black);
            REQUIRE(t);
        }
        REQUIRE(texManager.getTextCacheSize() == before + 1);
    }
}
