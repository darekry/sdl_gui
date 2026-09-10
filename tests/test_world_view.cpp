#include "../lib/catch_amalgamated.hpp"

#include "test_helper.hpp"
#include "../src/world_view.hpp"
#include "../src/button.hpp"
#include "../src/gui_manager.hpp"
#include "../src/widget_factory.hpp"

TEST_CASE("WorldView - construction", "[world_view]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("constructor defaults world to view size, camera at 0,0") {
        auto view = std::make_unique<WorldView>(manager, 10, 20, 400, 300);
        WorldView* ptr = view.get();
        manager.addElement(std::move(view));

        REQUIRE(ptr->getComponentTypeId() == ComponentType::WorldView);
        REQUIRE(ptr->getWorldWidth() == 400);
        REQUIRE(ptr->getWorldHeight() == 300);
        REQUIRE(ptr->getCamX() == 0);
        REQUIRE(ptr->getCamY() == 0);
        REQUIRE(ptr->getContent() != nullptr);
    }

    SECTION("setWorldSize stores size and clamps camera") {
        WorldView view(manager, 0, 0, 400, 300);
        view.setWorldSize(800, 600);
        REQUIRE(view.getWorldWidth() == 800);
        REQUIRE(view.getWorldHeight() == 600);
        view.setCamera(500, 500); // max is (400, 300)
        REQUIRE(view.getCamX() == 400);
        REQUIRE(view.getCamY() == 300);
        view.setWorldSize(2000, 2000);
        REQUIRE(view.getCamX() == 400);
        REQUIRE(view.getCamY() == 300);
    }

    SECTION("world smaller than view clamps camera to zero") {
        WorldView view(manager, 0, 0, 400, 300);
        view.setWorldSize(100, 100);
        view.setCamera(50, 50);
        REQUIRE(view.getCamX() == 0);
        REQUIRE(view.getCamY() == 0);
    }
}

TEST_CASE("WorldView - camera and coords", "[world_view]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();

    SECTION("setCamera moves content to negative offset") {
        auto view = std::make_unique<WorldView>(manager, 0, 0, 400, 300);
        WorldView* ptr = view.get();
        manager.addElement(std::move(view));

        ptr->setWorldSize(800, 600);
        ptr->setCamera(100, 150);
        REQUIRE(ptr->getCamX() == 100);
        REQUIRE(ptr->getCamY() == 150);
        REQUIRE(ptr->getContent()->getX() == -100);
        REQUIRE(ptr->getContent()->getY() == -150);
    }

    SECTION("worldToScreen/screenToWorld round-trip") {
        WorldView view(manager, 0, 0, 400, 300);
        view.setWorldSize(800, 600);
        view.setCamera(100, 150);
        SDL_Point s = view.worldToScreen(500, 400);
        REQUIRE(s.x == 400);
        REQUIRE(s.y == 250);
        SDL_Point w = view.screenToWorld(s.x, s.y);
        REQUIRE(w.x == 500);
        REQUIRE(w.y == 400);
    }

    SECTION("panBy accumulates and clamps") {
        WorldView view(manager, 0, 0, 400, 300);
        view.setWorldSize(800, 600);
        view.panBy(1000, 0); // max X is 400
        REQUIRE(view.getCamX() == 400);
        view.panBy(-500, 0);
        REQUIRE(view.getCamX() == 0);
    }

    SECTION("centerOn puts world point in view center") {
        WorldView view(manager, 0, 0, 400, 300);
        view.setWorldSize(800, 600);
        view.centerOn(500, 400);
        REQUIRE(view.getCamX() == 300);
        REQUIRE(view.getCamY() == 250);
    }

    SECTION("world child keeps world coords, screen pos follows camera") {
        auto view = std::make_unique<WorldView>(manager, 0, 0, 400, 300);
        WorldView* ptr = view.get();
        manager.addElement(std::move(view));
        ptr->setWorldSize(800, 600);

        auto btn = std::make_unique<Button>(manager, 500, 400, 60, 30, "U");
        Button* btnPtr = btn.get();
        ptr->addWorldChild(std::move(btn));
        REQUIRE(btnPtr->getX() == 500);
        REQUIRE(btnPtr->getY() == 400);

        ptr->setCamera(100, 150);
        SDL_Point abs = btnPtr->getAbsolutePosition();
        REQUIRE(abs.x == 400);
        REQUIRE(abs.y == 250);
    }

    SECTION("widget factory builds WorldView with world size props") {
        WidgetProps props;
        props.w = 400;
        props.h = 300;
        props.contentWidth = 800;
        props.contentHeight = 600;
        auto wv = WidgetFactory::create(manager, "WorldView", props);
        REQUIRE(wv);
        REQUIRE(wv->getComponentTypeId() == ComponentType::WorldView);
        auto* view = static_cast<WorldView*>(wv.get());
        REQUIRE(view->getWorldWidth() == 800);
        REQUIRE(view->getWorldHeight() == 600);
    }
}

TEST_CASE("WorldView - focus outline respects viewport clip", "[world_view][render]") {
    TestHelper helper;
    GUIManager& manager = helper.getManager();
    manager.setTheme(Theme::createDefaultTheme());
    manager.handleResize(320, 240);

    auto view = std::make_unique<WorldView>(manager, 50, 50, 200, 200);
    WorldView* viewPtr = view.get();
    manager.addElement(std::move(view));
    viewPtr->setWorldSize(400, 400);

    auto btn = std::make_unique<Button>(manager, 0, 80, 100, 40, "edge");
    Button* btnPtr = btn.get();
    btnPtr->setBorderRadius(ElementState::Normal, 0);
    viewPtr->addWorldChild(std::move(btn));

    // Kamera 50px w prawo: przycisk (abs x 0..100) wystaje 50px za lewy
    // brzeg viewportu (50..250). Górna krawędź obrysu: y = 50+80 = 130.
    viewPtr->setCamera(50, 0);
    manager.setKeyboardFocus(btnPtr);
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

    SECTION("focus outline outside viewport is clipped away") {
        // (49,130): na górnej krawędzi obrysu, ale 1px na lewo od viewportu.
        // Przed fixem renderFocusOverlay rysował bez clipa → focus-blue.
        auto px = readPixel(49, 130);
        bool isFocusBlue = (px[0] == 0 && px[1] == 120 && px[2] == 215);
        REQUIRE(!isFocusBlue);
    }

    SECTION("focus outline inside viewport still draws (control)") {
        // (60,130): ten sam obrys, ale wewnątrz viewportu.
        auto px = readPixel(60, 130);
        REQUIRE(px[0] == 0);
        REQUIRE(px[1] == 120);
        REQUIRE(px[2] == 215);
    }
}
