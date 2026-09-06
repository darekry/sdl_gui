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

TEST_CASE("TextureManager byte-budget LRU", "[texture_manager][lru]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    TextureManager& texManager = manager.getTextureManager();

    // renderCache(w, h) księguje dokładnie w*h*4 bajtów — determinystyczne.
    auto noop = [](SDL_Renderer*) {};
    constexpr uint64_t k1 = 101, k2 = 102, k3 = 103;
    constexpr size_t kEntry = 50u * 50u * 4u; // 10000

    SECTION("default budget and accounting") {
        REQUIRE(texManager.getByteBudget() == TextureManager::kDefaultByteBudget);
        REQUIRE(texManager.getBytesUsed() == 0);
        auto t1 = texManager.renderCache(k1, 50, 50, noop);
        auto t2 = texManager.renderCache(k2, 25, 100, noop);
        REQUIRE(t1);
        REQUIRE(t2);
        REQUIRE(texManager.getBytesUsed() == 2 * kEntry);
    }

    SECTION("overflowing insert evicts oldest dead entry") {
        texManager.setByteBudget(2 * kEntry + kEntry / 2); // 25000
        auto t1 = texManager.renderCache(k1, 50, 50, noop);
        auto t2 = texManager.renderCache(k2, 50, 50, noop);
        REQUIRE(texManager.getBytesUsed() == 2 * kEntry);
        t1.reset(); // k1 martwe, k2 żywe
        auto t3 = texManager.renderCache(k3, 50, 50, noop);
        REQUIRE(t3);
        REQUIRE(texManager.getRenderCacheSize() == 2);
        REQUIRE(texManager.getBytesUsed() == 2 * kEntry);
        // k1 wyrzucone: ponowne żądanie to miss (nowy wpis), k2 nietknięte.
        REQUIRE(texManager.getRenderCacheSize() == 2);
    }

    SECTION("live entries are never evicted (budget may overshoot)") {
        texManager.setByteBudget(2 * kEntry + kEntry / 2);
        auto t1 = texManager.renderCache(k1, 50, 50, noop);
        auto t2 = texManager.renderCache(k2, 50, 50, noop);
        auto t3 = texManager.renderCache(k3, 50, 50, noop);
        REQUIRE(t1);
        REQUIRE(t2);
        REQUIRE(t3);
        REQUIRE(texManager.getRenderCacheSize() == 3);
        REQUIRE(texManager.getBytesUsed() == 3 * kEntry);
    }

    SECTION("hit refreshes recency: touched dead entry survives") {
        texManager.setByteBudget(2 * kEntry + kEntry / 2);
        auto t1 = texManager.renderCache(k1, 50, 50, noop);
        auto t2 = texManager.renderCache(k2, 50, 50, noop);
        SDL_Texture* raw1 = t1.get();
        SDL_Texture* raw2 = t2.get();
        // Dotknij k1 (hit), potem oba martwe — k2 jest starsze.
        REQUIRE(texManager.renderCache(k1, 50, 50, noop).get() == raw1);
        t1.reset();
        t2.reset();
        auto t3 = texManager.renderCache(k3, 50, 50, noop);
        REQUIRE(t3);
        REQUIRE(texManager.getRenderCacheSize() == 2);
        // k1 przeżyło (ten sam wskaźnik), k2 wyrzucone (miss → nowy obiekt).
        REQUIRE(texManager.renderCache(k1, 50, 50, noop).get() == raw1);
        REQUIRE(texManager.renderCache(k2, 50, 50, noop).get() != raw2);
    }

    SECTION("shrinking the budget enforces it immediately") {
        auto t1 = texManager.renderCache(k1, 50, 50, noop);
        auto t2 = texManager.renderCache(k2, 50, 50, noop);
        SDL_Texture* raw2 = t2.get();
        t1.reset();
        t2.reset();
        REQUIRE(texManager.getBytesUsed() == 2 * kEntry);
        texManager.setByteBudget(kEntry);
        REQUIRE(texManager.getBytesUsed() == kEntry);
        REQUIRE(texManager.getRenderCacheSize() == 1);
        // Młodsze k2 przeżyło.
        REQUIRE(texManager.renderCache(k2, 50, 50, noop).get() == raw2);
    }

    SECTION("pruneUnused releases bytes of dead entries") {
        auto t1 = texManager.renderCache(k1, 50, 50, noop);
        REQUIRE(texManager.getBytesUsed() == kEntry);
        t1.reset();
        texManager.pruneUnused();
        REQUIRE(texManager.getBytesUsed() == 0);
        REQUIRE(texManager.getRenderCacheSize() == 0);
    }
}
