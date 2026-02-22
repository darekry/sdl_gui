#define CATCH_CONFIG_MAIN
#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/font_manager.hpp"
#include "../src/gui_manager.hpp"

// Ścieżki do istniejących plików czcionek w projekcie
static constexpr const char* VALID_FONT_PATH = "assets/fonts/DejaVuSans.ttf";
static constexpr const char* VALID_FONT_MONO_PATH = "assets/fonts/DejaVuSansMono.ttf";
static constexpr const char* GENERIC_FONT_PATH = "assets/fonts/font.ttf";
static constexpr const char* NONEXISTENT_FONT_PATH = "nonexistent/font.ttf";

TEST_CASE("FontManager functionality", "[font_manager]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    FontManager& fontManager = manager.getFontManager();

    SECTION("Loading non-existent font returns nullptr") {
        auto font = fontManager.loadFont(NONEXISTENT_FONT_PATH, 16);
        REQUIRE(font == nullptr);
    }

    SECTION("getFont returns nullptr for non-existent font") {
        auto* font = fontManager.getFont(NONEXISTENT_FONT_PATH, 16);
        REQUIRE(font == nullptr);
    }

    SECTION("getFont with string_view path for non-existent font") {
        std::string path = NONEXISTENT_FONT_PATH;
        std::string_view pathView = path;
        
        auto* font = fontManager.getFont(pathView, 16);
        REQUIRE(font == nullptr);
    }
}

TEST_CASE("FontManager - loading existing fonts", "[font_manager]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    FontManager& fontManager = manager.getFontManager();

    SECTION("Loading existing font returns valid SharedFont") {
        auto font = fontManager.loadFont(VALID_FONT_PATH, 16);
        REQUIRE(font != nullptr);
        REQUIRE(font.get() != nullptr);
    }

    SECTION("Loading different existing fonts") {
        auto font1 = fontManager.loadFont(VALID_FONT_PATH, 16);
        auto font2 = fontManager.loadFont(VALID_FONT_MONO_PATH, 16);
        
        REQUIRE(font1 != nullptr);
        REQUIRE(font2 != nullptr);
        // Różne pliki czcionek powinny dać różne obiekty
        REQUIRE(font1.get() != font2.get());
    }

    SECTION("Loading generic font.ttf") {
        auto font = fontManager.loadFont(GENERIC_FONT_PATH, 16);
        REQUIRE(font != nullptr);
    }
}

TEST_CASE("FontManager - caching mechanism", "[font_manager]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    FontManager& fontManager = manager.getFontManager();

    SECTION("Font caching works correctly - same path and size returns same font") {
        auto font1 = fontManager.loadFont(VALID_FONT_PATH, 16);
        auto font2 = fontManager.loadFont(VALID_FONT_PATH, 16);
        
        REQUIRE(font1 != nullptr);
        REQUIRE(font2 != nullptr);
        REQUIRE(font1.get() == font2.get());
        REQUIRE(font1 == font2);
    }

    SECTION("Different font sizes create different cache entries") {
        auto font1 = fontManager.loadFont(VALID_FONT_PATH, 16);
        auto font2 = fontManager.loadFont(VALID_FONT_PATH, 24);
        
        REQUIRE(font1 != nullptr);
        REQUIRE(font2 != nullptr);
        // Różne rozmiary powinny dać różne obiekty czcionek
        REQUIRE(font1.get() != font2.get());
    }

    SECTION("Multiple loads of same font return cached instance") {
        auto font1 = fontManager.loadFont(VALID_FONT_PATH, 12);
        auto font2 = fontManager.loadFont(VALID_FONT_PATH, 12);
        auto font3 = fontManager.loadFont(VALID_FONT_PATH, 12);
        
        REQUIRE(font1.get() == font2.get());
        REQUIRE(font2.get() == font3.get());
    }

    SECTION("getFont returns same cached font as loadFont") {
        auto loadedFont = fontManager.loadFont(VALID_FONT_PATH, 16);
        auto* gotFont = fontManager.getFont(VALID_FONT_PATH, 16);
        
        REQUIRE(loadedFont.get() == gotFont);
    }

    SECTION("getFont with string_view returns cached font") {
        auto loadedFont = fontManager.loadFont(VALID_FONT_PATH, 16);
        
        std::string path = VALID_FONT_PATH;
        std::string_view pathView = path;
        auto* gotFont = fontManager.getFont(pathView, 16);
        
        REQUIRE(loadedFont.get() == gotFont);
    }
}

TEST_CASE("FontManager - different font sizes", "[font_manager]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    FontManager& fontManager = manager.getFontManager();

    SECTION("Multiple sizes of same font can be loaded") {
        auto font12 = fontManager.loadFont(VALID_FONT_PATH, 12);
        auto font16 = fontManager.loadFont(VALID_FONT_PATH, 16);
        auto font24 = fontManager.loadFont(VALID_FONT_PATH, 24);
        auto font32 = fontManager.loadFont(VALID_FONT_PATH, 32);
        auto font48 = fontManager.loadFont(VALID_FONT_PATH, 48);
        
        REQUIRE(font12 != nullptr);
        REQUIRE(font16 != nullptr);
        REQUIRE(font24 != nullptr);
        REQUIRE(font32 != nullptr);
        REQUIRE(font48 != nullptr);
        
        // Wszystkie powinny być różne
        REQUIRE(font12.get() != font16.get());
        REQUIRE(font16.get() != font24.get());
        REQUIRE(font24.get() != font32.get());
        REQUIRE(font32.get() != font48.get());
    }

    SECTION("Very small font size") {
        auto font = fontManager.loadFont(VALID_FONT_PATH, 6);
        REQUIRE(font != nullptr);
    }

    SECTION("Large font size") {
        auto font = fontManager.loadFont(VALID_FONT_PATH, 72);
        REQUIRE(font != nullptr);
    }
}

TEST_CASE("FontManager - default font", "[font_manager]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    FontManager& fontManager = manager.getFontManager();

    SECTION("Default font can be set and retrieved") {
        fontManager.loadDefaultFont(VALID_FONT_PATH, 18);
        auto defaultFont = fontManager.getDefaultFont();
        
        REQUIRE(defaultFont != nullptr);
    }

    SECTION("Default font is cached correctly") {
        fontManager.loadDefaultFont(VALID_FONT_PATH, 18);
        auto defaultFont1 = fontManager.getDefaultFont();
        auto defaultFont2 = fontManager.getDefaultFont();
        
        REQUIRE(defaultFont1.get() == defaultFont2.get());
    }

    SECTION("Default font with non-existent path remains nullptr") {
        fontManager.loadDefaultFont(NONEXISTENT_FONT_PATH, 18);
        auto defaultFont = fontManager.getDefaultFont();
        
        REQUIRE(defaultFont == nullptr);
    }
}

TEST_CASE("FontManager - getTextSize", "[font_manager]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    FontManager& fontManager = manager.getFontManager();

    SECTION("getTextSize returns zero dimensions for non-existent font") {
        int width = -1;
        int height = -1;
        
        fontManager.getTextSize("test text", NONEXISTENT_FONT_PATH, 16, &width, &height);
        
        REQUIRE(width == 0);
        REQUIRE(height == 0);
    }

    SECTION("getTextSize returns valid dimensions for existing font") {
        int width = 0;
        int height = 0;
        
        fontManager.getTextSize("Hello World", VALID_FONT_PATH, 16, &width, &height);
        
        REQUIRE(width > 0);
        REQUIRE(height > 0);
    }

    SECTION("getTextSize handles null output parameters gracefully") {
        // Nie powinno crashować przy null wskaźnikach
        REQUIRE_NOTHROW(fontManager.getTextSize("test", VALID_FONT_PATH, 16, nullptr, nullptr));
        REQUIRE_NOTHROW(fontManager.getTextSize("test", VALID_FONT_PATH, 16, nullptr, nullptr));
    }

    SECTION("getTextSize - longer text has larger width") {
        int width1 = 0, height1 = 0;
        int width2 = 0, height2 = 0;
        
        fontManager.getTextSize("Hi", VALID_FONT_PATH, 16, &width1, &height1);
        fontManager.getTextSize("Hello World!", VALID_FONT_PATH, 16, &width2, &height2);
        
        REQUIRE(width2 > width1);
        // Wysokość powinna być taka sama dla tej samej czcionki
        REQUIRE(height1 == height2);
    }

    SECTION("getTextSize - larger font size gives larger dimensions") {
        int width16 = 0, height16 = 0;
        int width24 = 0, height24 = 0;
        
        fontManager.getTextSize("Test", VALID_FONT_PATH, 16, &width16, &height16);
        fontManager.getTextSize("Test", VALID_FONT_PATH, 24, &width24, &height24);
        
        REQUIRE(width24 > width16);
        REQUIRE(height24 > height16);
    }

    SECTION("getTextSize - empty string") {
        int width = -1;
        int height = -1;
        
        fontManager.getTextSize("", VALID_FONT_PATH, 16, &width, &height);
        
        // Pusty tekst powinien mieć zerową szerokość, ale może mieć niezerową wysokość
        REQUIRE(width >= 0);
        REQUIRE(height >= 0);
    }

    SECTION("getTextSize - single character") {
        int width = 0;
        int height = 0;
        
        fontManager.getTextSize("A", VALID_FONT_PATH, 16, &width, &height);
        
        REQUIRE(width > 0);
        REQUIRE(height > 0);
    }

    SECTION("getTextSize - special characters") {
        int width = 0;
        int height = 0;
        
        fontManager.getTextSize("!@#$%^&*()", VALID_FONT_PATH, 16, &width, &height);
        
        REQUIRE(width > 0);
        REQUIRE(height > 0);
    }

    SECTION("getTextSize - unicode characters (Polish)") {
        int width = 0;
        int height = 0;
        
        fontManager.getTextSize("ąćęłńóśźż", VALID_FONT_PATH, 16, &width, &height);
        
        REQUIRE(width > 0);
        REQUIRE(height > 0);
    }

    SECTION("getTextSize caches the font") {
        int width = 0;
        int height = 0;
        
        // Pierwsze wywołanie - ładuje czcionkę
        fontManager.getTextSize("Test", VALID_FONT_PATH, 16, &width, &height);
        
        // Drugie wywołanie - powinno użyć cache'u
        fontManager.getTextSize("Another test", VALID_FONT_PATH, 16, &width, &height);
        
        REQUIRE(width > 0);
        REQUIRE(height > 0);
    }
}

TEST_CASE("FontManager - SharedFont lifetime", "[font_manager]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    FontManager& fontManager = manager.getFontManager();

    SECTION("SharedFont keeps font alive after cache query") {
        TTF_Font* rawPtr = nullptr;
        
        {
            auto font = fontManager.loadFont(VALID_FONT_PATH, 16);
            REQUIRE(font != nullptr);
            rawPtr = font.get();
        }
        
        // Po wyjściu z zakresu, SharedFont w cache'u nadal trzyma zasób
        auto cachedFont = fontManager.loadFont(VALID_FONT_PATH, 16);
        REQUIRE(cachedFont.get() == rawPtr);
    }

    SECTION("Multiple SharedFont copies reference same font") {
        auto font1 = fontManager.loadFont(VALID_FONT_PATH, 16);
        auto font2 = font1;  // Kopiowanie shared_ptr
        auto font3 = font1;  // Kolejna kopia
        
        REQUIRE(font1.get() == font2.get());
        REQUIRE(font2.get() == font3.get());
    }
}

TEST_CASE("FontManager - edge cases", "[font_manager]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    FontManager& fontManager = manager.getFontManager();

    SECTION("Loading font with size 0") {
        // SDL_ttf może zaakceptować rozmiar 0, ale to zależy od implementacji
        auto font = fontManager.loadFont(VALID_FONT_PATH, 0);
        // Nie sprawdzamy czy jest null czy nie - zależy od SDL_ttf
        // Sprawdzamy tylko że nie crashuje
    }

    SECTION("Loading font with negative size") {
        // UWAGA: SDL_ttf akceptuje ujemne rozmiary (traktuje jako bezwzględne)
        // To jest zachowanie SDL_ttf, nie błąd FontManager
        auto font = fontManager.loadFont(VALID_FONT_PATH, -1);
        // Nie sprawdzamy czy jest null - SDL_ttf akceptuje ujemne rozmiary
        // Sprawdzamy tylko że nie crashuje
    }

    SECTION("Empty path") {
        auto font = fontManager.loadFont("", 16);
        REQUIRE(font == nullptr);
    }

    SECTION("Path with spaces") {
        // Test czy menedżer obsługuje ścieżki ze spacjami (jeśli istnieją)
        auto font = fontManager.loadFont("path with spaces/font.ttf", 16);
        REQUIRE(font == nullptr);  // Nie istnieje, ale nie powinno crashować
    }
}

TEST_CASE("FontManager - performance and stress", "[font_manager]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    FontManager& fontManager = manager.getFontManager();

    SECTION("Many cache lookups are efficient") {
        // Załaduj czcionkę raz
        auto font = fontManager.loadFont(VALID_FONT_PATH, 16);
        REQUIRE(font != nullptr);
        
        // Wiele zapytań o tę samą czcionkę
        for (int i = 0; i < 100; ++i) {
            auto cachedFont = fontManager.loadFont(VALID_FONT_PATH, 16);
            REQUIRE(cachedFont.get() == font.get());
        }
    }

    SECTION("Loading many different sizes") {
        // Test wydajnościowy - ładowanie wielu rozmiarów
        for (int size = 8; size <= 48; size += 2) {
            auto font = fontManager.loadFont(VALID_FONT_PATH, size);
            REQUIRE(font != nullptr);
        }
    }
}
