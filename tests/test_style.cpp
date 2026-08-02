#include "../lib/catch_amalgamated.hpp"
#include "../src/style.hpp"

TEST_CASE("Style - default state", "[style]") {
    Style s;
    SECTION("all fields are empty by default") {
        REQUIRE_FALSE(s.backgroundColor.has_value());
        REQUIRE_FALSE(s.textColor.has_value());
        REQUIRE_FALSE(s.texture.has_value());
        REQUIRE_FALSE(s.borderColor.has_value());
        REQUIRE_FALSE(s.borderWidth.has_value());
        REQUIRE_FALSE(s.borderRadius.has_value());
        REQUIRE_FALSE(s.fontSize.has_value());
        REQUIRE_FALSE(s.fontName.has_value());
    }
}

TEST_CASE("Style - mergeWith", "[style]") {
    SECTION("fills missing fields from base") {
        Style base;
        base.backgroundColor = SDL_Color{10, 20, 30, 255};
        base.borderWidth = 2;
        base.fontSize = 16;

        Style s;
        s.textColor = SDL_Color{1, 2, 3, 255};
        s.mergeWith(base);

        REQUIRE(s.backgroundColor == base.backgroundColor);
        REQUIRE(s.borderWidth == base.borderWidth);
        REQUIRE(s.fontSize == base.fontSize);
        REQUIRE(s.textColor == SDL_Color{1, 2, 3, 255});
    }

    SECTION("does not override existing values") {
        Style base;
        base.backgroundColor = SDL_Color{10, 20, 30, 255};

        Style s;
        s.backgroundColor = SDL_Color{99, 99, 99, 255};
        s.mergeWith(base);

        REQUIRE(s.backgroundColor == SDL_Color{99, 99, 99, 255});
    }

    SECTION("merging with empty base is a no-op") {
        Style base;
        Style s;
        s.borderRadius = 8;
        s.mergeWith(base);

        REQUIRE(s.borderRadius == 8);
        REQUIRE_FALSE(s.borderColor.has_value());
    }
}

TEST_CASE("Style - equality", "[style]") {
    SECTION("identical styles are equal") {
        Style a;
        a.backgroundColor = SDL_Color{1, 2, 3, 4};
        a.borderWidth = 2;
        a.fontSize = 16;
        a.fontName = "DejaVu";

        Style b = a;
        REQUIRE(a == b);
        REQUIRE_FALSE(a != b);
    }

    SECTION("default styles are equal") {
        Style a;
        Style b;
        REQUIRE(a == b);
    }

    SECTION("different background colors are not equal") {
        Style a;
        Style b;
        a.backgroundColor = SDL_Color{1, 2, 3, 4};
        b.backgroundColor = SDL_Color{5, 6, 7, 8};
        REQUIRE(a != b);
    }

    SECTION("presence vs absence of a field differs") {
        Style a;
        Style b;
        b.backgroundColor = SDL_Color{1, 2, 3, 4};
        REQUIRE(a != b);
    }

    SECTION("different border widths are not equal") {
        Style a;
        Style b;
        a.borderWidth = 1;
        b.borderWidth = 2;
        REQUIRE(a != b);
    }

    SECTION("different border radii are not equal") {
        Style a;
        Style b;
        a.borderRadius = 4;
        b.borderRadius = 8;
        REQUIRE(a != b);
    }

    SECTION("different font sizes are not equal") {
        Style a;
        Style b;
        a.fontSize = 12;
        b.fontSize = 16;
        REQUIRE(a != b);
    }

    SECTION("different font names are not equal") {
        Style a;
        Style b;
        a.fontName = "A";
        b.fontName = "B";
        REQUIRE(a != b);
    }

    SECTION("texture equality compares pointers") {
        auto noop = [](SDL_Texture*) {};
        Style a;
        Style b;
        SharedTexture texA(reinterpret_cast<SDL_Texture*>(0x1), noop);
        SharedTexture texB(reinterpret_cast<SDL_Texture*>(0x1), noop);
        a.texture = texA;
        b.texture = texB;
        REQUIRE(a == b);

        SharedTexture texC(reinterpret_cast<SDL_Texture*>(0x2), noop);
        b.texture = texC;
        REQUIRE(a != b);
    }
}

TEST_CASE("Style - SDL_Color comparison operator", "[style]") {
    SECTION("equal colors") {
        REQUIRE((SDL_Color{1, 2, 3, 4} == SDL_Color{1, 2, 3, 4}));
    }

    SECTION("different colors") {
        REQUIRE_FALSE((SDL_Color{1, 2, 3, 4} == SDL_Color{1, 2, 3, 5}));
        REQUIRE_FALSE((SDL_Color{1, 2, 3, 4} == SDL_Color{2, 2, 3, 4}));
        REQUIRE_FALSE((SDL_Color{1, 2, 3, 4} == SDL_Color{1, 3, 3, 4}));
        REQUIRE_FALSE((SDL_Color{1, 2, 3, 4} == SDL_Color{1, 2, 4, 4}));
    }
}

TEST_CASE("Style - element states", "[style]") {
    SECTION("all four states are distinct") {
        REQUIRE(ElementState::Normal != ElementState::Hover);
        REQUIRE(ElementState::Hover != ElementState::Pressed);
        REQUIRE(ElementState::Pressed != ElementState::Disabled);
    }
}
