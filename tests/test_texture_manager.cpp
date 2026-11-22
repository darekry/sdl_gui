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

    SECTION("createTextureFromText is uncached") {
        const size_t before = texManager.getTextCacheSize();
        auto t1 = texManager.createTextureFromText("shared", font16, black);
        auto t2 = texManager.createTextureFromText("shared", font16, black);
        REQUIRE(t1);
        REQUIRE(t2);
        REQUIRE(t2.get() != t1.get());
        REQUIRE(texManager.getTextCacheSize() == before);
    }

    SECTION("acquireTextTexture shares one texture between leases") {
        const size_t before = texManager.getTextCacheSize();
        auto l1 = texManager.acquireTextTexture("shared", font16, black);
        auto l2 = texManager.acquireTextTexture("shared", font16, black);
        REQUIRE(l1);
        REQUIRE(l2);
        REQUIRE(l1.get() == l2.get());
        REQUIRE(texManager.getTextCacheSize() == before + 1);
    }

    SECTION("releasing one lease keeps the entry, the last one removes it") {
        const size_t before = texManager.getTextCacheSize();
        auto l1 = texManager.acquireTextTexture("lease", font16, black);
        auto l2 = texManager.acquireTextTexture("lease", font16, black);
        REQUIRE(l1.get() == l2.get());
        l1.reset();
        REQUIRE_FALSE(l1);
        REQUIRE(texManager.getTextCacheSize() == before + 1);
        l2.reset();
        REQUIRE(texManager.getTextCacheSize() == before);
    }

    SECTION("color is part of the key") {
        auto l1 = texManager.acquireTextTexture("colorkey", font16, black);
        auto l2 = texManager.acquireTextTexture("colorkey", font16, red);
        REQUIRE(l1);
        REQUIRE(l2);
        REQUIRE(l2.get() != l1.get());
    }

    SECTION("font is part of the key") {
        auto l1 = texManager.acquireTextTexture("fontkey", font16, black);
        auto l2 = texManager.acquireTextTexture("fontkey", font24, black);
        REQUIRE(l1);
        REQUIRE(l2);
        REQUIRE(l2.get() != l1.get());
    }

    SECTION("different texts do not collide") {
        auto l1 = texManager.acquireTextTexture("alpha", font16, black);
        auto l2 = texManager.acquireTextTexture("beta", font16, black);
        REQUIRE(l1);
        REQUIRE(l2);
        REQUIRE(l2.get() != l1.get());
    }

    SECTION("grid-scale sharing: N leases of identical cells, 1 entry") {
        const size_t before = texManager.getTextCacheSize();
        std::vector<TextureManager::TextLease> leases;
        for (int i = 0; i < 100; ++i) {
            leases.push_back(texManager.acquireTextTexture("cell", font16, black));
            REQUIRE(leases.back());
        }
        REQUIRE(texManager.getTextCacheSize() == before + 1);
        leases.clear();
        REQUIRE(texManager.getTextCacheSize() == before);
    }
}

TEST_CASE("TextureManager render cache weak lifetime", "[texture_manager][render_cache]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    TextureManager& texManager = manager.getTextureManager();

    auto noop = [](SDL_Renderer*) {};
    constexpr uint64_t k1 = 201;
    constexpr uint64_t k2 = 202;

    SECTION("identical widgets share one texture and one entry") {
        const size_t before = texManager.getRenderCacheSize();
        auto t1 = texManager.renderCache(k1, 50, 50, noop);
        auto t2 = texManager.renderCache(k1, 50, 50, noop);
        REQUIRE(t1);
        REQUIRE(t1.get() == t2.get());
        REQUIRE(texManager.getRenderCacheSize() == before + 1);
    }

    SECTION("one holder destroyed -> entry stays; last -> release removes it") {
        const size_t before = texManager.getRenderCacheSize();
        auto t1 = texManager.renderCache(k1, 50, 50, noop);
        auto t2 = texManager.renderCache(k2, 50, 50, noop);
        REQUIRE(t1);
        REQUIRE(t2);
        REQUIRE(texManager.getRenderCacheSize() == before + 2);

        t1.reset();
        texManager.releaseRenderCache(k1);
        REQUIRE(texManager.getRenderCacheSize() == before + 1); // k2 still alive

        t2.reset();
        texManager.releaseRenderCache(k2);
        REQUIRE(texManager.getRenderCacheSize() == before);
    }

    SECTION("release with a live holder is a no-op") {
        const size_t before = texManager.getRenderCacheSize();
        auto t1 = texManager.renderCache(k1, 50, 50, noop);
        REQUIRE(t1);
        texManager.releaseRenderCache(k1);
        REQUIRE(texManager.getRenderCacheSize() == before + 1);
        REQUIRE(texManager.renderCache(k1, 50, 50, noop).get() == t1.get());
    }

    SECTION("expired entry re-renders on the next request") {
        const size_t before = texManager.getRenderCacheSize();
        auto t1 = texManager.renderCache(k1, 50, 50, noop);
        REQUIRE(t1);
        t1.reset();
        texManager.releaseRenderCache(k1);
        REQUIRE(texManager.getRenderCacheSize() == before);

        bool redrawn = false;
        auto t2 = texManager.renderCache(k1, 50, 50, [&redrawn](SDL_Renderer*) { redrawn = true; });
        REQUIRE(t2);
        REQUIRE(redrawn);
        REQUIRE(texManager.getRenderCacheSize() == before + 1);
    }

    SECTION("path cache stays strong: local drop still hits") {
        auto tex = helper.makeStubTexture(32, 32);
        texManager.addTexture("strong_key", tex);
        tex.reset();
        REQUIRE(texManager.hasTexture("strong_key"));
        REQUIRE(texManager.getTexture("strong_key") != nullptr);
    }

    SECTION("clearCache drops metadata; widget-held textures survive") {
        auto held = texManager.renderCache(k1, 50, 50, noop);
        REQUIRE(held);
        texManager.clearCache();
        REQUIRE(texManager.getRenderCacheSize() == 0);
        REQUIRE(held); // strong ref still valid
    }
}
