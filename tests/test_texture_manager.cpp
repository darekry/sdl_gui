#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/texture_manager.hpp"
#include "../src/font_manager.hpp"
#include "../src/gui_manager.hpp"

TEST_CASE("TextureManager functionality", "[texture_manager]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    TextureManager& texManager = manager.getTextureManager();

    SECTION("Load texture from file") {
        auto tex = texManager.loadTexture("assets/button1.png");
        REQUIRE(tex != nullptr);
        
        int w, h;
        SDL_QueryTexture(tex.get(), nullptr, nullptr, &w, &h);
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
        texManager.queryTexture("assets/button2.png", w1, h1);
        
        REQUIRE(texManager.hasTexture("assets/button2.png"));
        
        int w2, h2;
        texManager.queryTexture("assets/button2.png", w2, h2);
        
        REQUIRE(w1 == w2);
        REQUIRE(h1 == h2);
    }
}
