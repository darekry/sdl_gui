#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"
#include "test_helper.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "panel.hpp"
#include "button.hpp"
#include "checkbox.hpp"
#include "constants.hpp"

// Regression: widgets must actually render pixels (ScopedRenderTarget clip restore
// used to enable a 0x0 clip rect and blank the whole frame).
TEST_CASE("Rendering produces opaque widget pixels", "[render][pixel]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    manager.setTheme(Theme::createDefaultTheme());
    manager.handleResize(320, 240);

    SDL_Color bg = {45, 48, 58, 255};
    auto panel = std::make_unique<Panel>(manager, 10, 10, 200, 100);
    panel->setBackgroundColor(ElementState::Normal, bg);
    panel->setBorderRadius(ElementState::Normal, 8);
    manager.addElement(std::move(panel));

    auto btn = std::make_unique<Button>(manager, 30, 130, 120, 40, "Test");
    btn->setBackgroundColor(ElementState::Normal, {200, 100, 50, 255});
    manager.addElement(std::move(btn));

    manager.update();
    manager.cleanup();
    manager.render();

    auto readPixel = [&](int x, int y) {
        SDL_Rect r{x, y, 1, 1};
        SDL_Surface* surf = SDL_RenderReadPixels(helper.getRenderer(), &r);
        REQUIRE(surf != nullptr);
        Uint8* p = (Uint8*)surf->pixels;
        auto result = std::array<Uint8, 4>{p[0], p[1], p[2], p[3]};
        SDL_DestroySurface(surf);
        return result;
    };

    SECTION("panel center matches its background color") {
        auto px = readPixel(110, 60);
        REQUIRE(px[0] == bg.r);
        REQUIRE(px[1] == bg.g);
        REQUIRE(px[2] == bg.b);
    }

    SECTION("button is opaque and colored (pixel away from label text)") {
        auto px = readPixel(45, 145);
        REQUIRE(px[3] > 200);
        REQUIRE(px[0] == 200);
        REQUIRE(px[1] == 100);
    }

    SECTION("empty area stays transparent (no clip bleed)") {
        auto px = readPixel(250, 200);
        REQUIRE(px[3] == 0);
    }
}

// Punkt 6, plaster 6 (rotacja/blit): obrys fokusu rotuje się razem z treścią
// (osobna tekstura m_focusTexture blitowana z tą samą rotacją), ale NIE jest
// wpiekany w cache treści. Panel 120x60, obrót 90° CW: lokalna górna krawędź
// (y=0, x 0..119) ląduje na kolumnie x=189 (ostatniej wewnątrz rotowanego
// blita 60x120 na środku (160,120)). Punkt (189,120) leży na rotowanym
// obrysie, ale W ŚRODKU nierotowanego AABB — odróżnia outline rotowany od
// osiowego (AABB nie dotyka tego piksela).
TEST_CASE("Focus outline rotates with content", "[render][pixel][rotation]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    manager.setTheme(Theme::createDefaultTheme());
    manager.handleResize(320, 240);

    auto panel = std::make_unique<Panel>(manager, 100, 90, 120, 60);
    Panel* panelPtr = panel.get();
    panelPtr->setBackgroundColor(ElementState::Normal, {60, 60, 70, 255});
    panelPtr->setBorderRadius(ElementState::Normal, 0);
    panelPtr->setBorder(ElementState::Normal, {0, 0, 0, 0}, 0);
    panelPtr->setRotation(90.0);
    manager.addElement(std::move(panel));

    auto readPixel = [&](int x, int y) {
        SDL_Rect r{x, y, 1, 1};
        SDL_Surface* surf = SDL_RenderReadPixels(helper.getRenderer(), &r);
        REQUIRE(surf != nullptr);
        Uint8* p = (Uint8*)surf->pixels;
        auto result = std::array<Uint8, 4>{p[0], p[1], p[2], p[3]};
        SDL_DestroySurface(surf);
        return result;
    };

    SECTION("rotated focused panel draws rotated focus outline") {
        manager.setKeyboardFocus(panelPtr);
        manager.update();
        manager.cleanup();
        manager.render();
        auto px = readPixel(189, 120);
        REQUIRE(px[0] == constants::kFocusOutlineColor.r);
        REQUIRE(px[1] == constants::kFocusOutlineColor.g);
        REQUIRE(px[2] == constants::kFocusOutlineColor.b);
        REQUIRE(px[3] == constants::kFocusOutlineColor.a);
    }

    SECTION("same pixel shows panel background without focus (control)") {
        manager.update();
        manager.cleanup();
        manager.render();
        auto px = readPixel(189, 120);
        REQUIRE(px[0] == 60);
        REQUIRE(px[1] == 60);
        REQUIRE(px[2] == 70);
        REQUIRE(px[3] == 255);
    }
}

// Direct-dziecko rotowanego rodzica nie da się wpiec w jego teksturę
// (rysuje w absolutnych współrzędnych co klatkę) — renderuje się osobno.
// Wcześniej było niewidzialne (wpieczony stub draw()).
class DirectRedElement : public GUIElement {
public:
    DirectRedElement(GUIManager& manager, int x, int y, int w, int h)
        : GUIElement(manager, x, y, w, h) {}
    ComponentType getComponentTypeId() const override { return ComponentType::Unknown; }
    bool wantsDirectRender() const override { return true; }
    void drawDirect(SDL_Renderer* renderer) override {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        const auto abs = getAbsolutePosition();
        SDL_FRect r{static_cast<float>(abs.x), static_cast<float>(abs.y),
                    static_cast<float>(getWidth()), static_cast<float>(getHeight())};
        SDL_RenderFillRect(renderer, &r);
    }
};

TEST_CASE("Direct child of rotated parent renders", "[render][pixel][rotation]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    manager.setTheme(Theme::createDefaultTheme());
    manager.handleResize(320, 240);

    auto parent = std::make_unique<Panel>(manager, 50, 50, 200, 200);
    Panel* parentPtr = parent.get();
    parentPtr->setBackgroundColor(ElementState::Normal, {0, 0, 255, 255});
    parentPtr->setRotation(30.0);
    auto child = std::make_unique<DirectRedElement>(manager, 100, 100, 40, 40);
    parentPtr->addChild(std::move(child));
    manager.addElement(std::move(parent));

    manager.update();
    manager.cleanup();
    manager.render();

    // Dziecko ma abs (150,150,40,40) — rodzic na (50,50), dziecko lokalnie (100,100).
    SDL_Rect r{170, 170, 1, 1};
    SDL_Surface* surf = SDL_RenderReadPixels(helper.getRenderer(), &r);
    REQUIRE(surf != nullptr);
    Uint8* p = (Uint8*)surf->pixels;
    auto px = std::array<Uint8, 4>{p[0], p[1], p[2], p[3]};
    SDL_DestroySurface(surf);

    // Środek dziecka: czerwień direct-render, nie niebieskie tło rodzica.
    REQUIRE(px[3] > 200);
    REQUIRE(px[0] == 255);
    REQUIRE(px[1] == 0);
    REQUIRE(px[2] == 0);
}

// Win9x-unifikacja: Checkbox na domyślnym (Windows95) motywie rysuje białe
// pudełko z fazą Sunken. Box 30x30 na (10,10): środek (25,25) biały,
// róg zewnętrzny TL (10,10) szary (Shadow), róg BR (39,39) biały (Highlight),
// róg wewnętrzny TL (11,11) czarny (DarkShadow) — odróżnia Sunken od
// płaskiego białego wypełnienia.
TEST_CASE("Checkbox renders white sunken box", "[render][pixel][theme]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    manager.setTheme(Theme::createDefaultTheme());
    manager.handleResize(320, 240);

    auto box = std::make_unique<Checkbox>(manager, 10, 10, 30, 30);
    manager.addElement(std::move(box));

    manager.update();
    manager.cleanup();
    manager.render();

    auto readPixel = [&](int x, int y) {
        SDL_Rect r{x, y, 1, 1};
        SDL_Surface* surf = SDL_RenderReadPixels(helper.getRenderer(), &r);
        REQUIRE(surf != nullptr);
        Uint8* p = (Uint8*)surf->pixels;
        auto result = std::array<Uint8, 4>{p[0], p[1], p[2], p[3]};
        SDL_DestroySurface(surf);
        return result;
    };

    auto center = readPixel(25, 25);
    REQUIRE(center[3] > 200);
    REQUIRE(center[0] == 255);
    REQUIRE(center[1] == 255);
    REQUIRE(center[2] == 255);

    auto outerTL = readPixel(10, 10);
    REQUIRE(outerTL[3] > 200);
    REQUIRE(outerTL[0] == 128);
    REQUIRE(outerTL[1] == 128);
    REQUIRE(outerTL[2] == 128);

    auto innerTL = readPixel(11, 11);
    REQUIRE(innerTL[3] > 200);
    REQUIRE(innerTL[0] == 0);
    REQUIRE(innerTL[1] == 0);
    REQUIRE(innerTL[2] == 0);

    auto outerBR = readPixel(39, 39);
    REQUIRE(outerBR[3] > 200);
    REQUIRE(outerBR[0] == 255);
    REQUIRE(outerBR[1] == 255);
    REQUIRE(outerBR[2] == 255);
}
