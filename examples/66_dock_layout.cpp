/**
 * @file 66_dock_layout.cpp
 * @brief DockLayout: top/bottom bars, left sidebar, fill center. Resize the window.
 */

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "panel.hpp"
#include "label.hpp"
#include "button.hpp"
#include "layout.hpp"

#include "std.hpp"

static void styleBox(Panel* p, SDL_Color bg) {
    p->setBackgroundColor(ElementState::Normal, bg);
    p->setBorder(ElementState::Normal, {98, 114, 164, 255}, 1);
    p->setBorderRadius(ElementState::Normal, 6);
}

int main(int, char**) {
    try {
        SDLApp app("DockLayout - resize the window", 800, 600, true);
        SDL_Renderer* renderer = app.getRenderer();
        int ww = 0, wh = 0;
        app.getWindowSize(ww, wh);
        GUIManager guiManager(renderer, Viewport{ww, wh});
        guiManager.setTheme(Theme::createDefaultTheme());

        // Root fills the window (Anchor) and docks its children (DockLayout).
        auto root = std::make_unique<Panel>(guiManager, 0, 0, ww, wh);
        root->setAnchor(Anchor::fill(0));
        root->setLayoutManager(std::make_unique<DockLayout>(4, 8, 8, 8, 8));
        Panel* rootRaw = root.get();
        guiManager.addElement(std::move(root));

        auto top = std::make_unique<Panel>(guiManager, 0, 0, 100, 50);
        top->setDock(Dock::Top);
        styleBox(top.get(), {58, 64, 84, 255});
        auto topLabel = std::make_unique<Label>(guiManager, 12, 14, "Top bar (dock=top, h=50)", 16);
        top->addChild(std::move(topLabel));
        rootRaw->addChild(std::move(top));

        auto bottom = std::make_unique<Panel>(guiManager, 0, 0, 100, 40);
        bottom->setDock(Dock::Bottom);
        styleBox(bottom.get(), {58, 64, 84, 255});
        auto bottomLabel = std::make_unique<Label>(guiManager, 12, 10, "Bottom bar (dock=bottom, h=40)", 16);
        bottom->addChild(std::move(bottomLabel));
        rootRaw->addChild(std::move(bottom));

        auto left = std::make_unique<Panel>(guiManager, 0, 0, 140, 100);
        left->setDock(Dock::Left);
        styleBox(left.get(), {48, 54, 74, 255});
        auto leftLabel = std::make_unique<Label>(guiManager, 12, 12, "Sidebar (w=140)", 16);
        left->addChild(std::move(leftLabel));
        rootRaw->addChild(std::move(left));

        auto center = std::make_unique<Panel>(guiManager, 0, 0, 100, 100);
        center->setDock(Dock::Fill);
        styleBox(center.get(), {40, 44, 58, 255});
        auto centerLabel = std::make_unique<Label>(guiManager, 12, 12, "Fill: reszta po pasach", 16);
        auto centerRef = guiManager.makeRef(centerLabel.get());
        center->addChild(std::move(centerLabel));
        auto btn = std::make_unique<Button>(guiManager, 12, 44, 160, 36, "Klik");
        btn->setOnClickCallback([centerRef](GUIElement*) {
            if (centerRef) centerRef->setText("Klik!");
        });
        center->addChild(std::move(btn));
        rootRaw->addChild(std::move(center));

        guiManager.setResizeCallback([centerRef](int w, int h) {
            if (centerRef) {
                centerRef->setText("Fill: okno " + std::to_string(w) + "x" + std::to_string(h));
            }
        });

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            Uint64 frameStart = SDL_GetTicks();
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) {
                    quit = true;
                    break;
                }
                if (e.type == SDL_EVENT_WINDOW_RESIZED) {
                    guiManager.handleResize(e.window.data1, e.window.data2);
                    continue;
                }
                guiManager.processEvent(e);
            }
            guiManager.update();
            guiManager.cleanup();
            SDL_SetRenderDrawColor(renderer, 40, 42, 54, 255);
            SDL_RenderClear(renderer);
            guiManager.render();
            SDL_RenderPresent(renderer);
            app.endFrame(frameStart);
        }
    } catch (const std::runtime_error& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
