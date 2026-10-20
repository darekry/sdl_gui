/**
 * @file 68_menu_bar.cpp
 * @brief Classic Win32-style menu bar: mnemonics, submenus, checks, shortcuts.
 *
 * Alt+F / F10 opens the menu bar, Left/Right switch menus, Up/Down navigate,
 * Enter activates, Esc closes one level. Shortcuts (Ctrl+S/Q...) fire while
 * menus are closed. The menu bar sits at (0,0); app content lives in a Panel
 * anchored below it (Anchor::pinned with topMargin = bar height).
 */

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "menu_bar.hpp"
#include "menu_item.hpp"
#include "panel.hpp"
#include "label.hpp"

#include "std.hpp"

int main(int, char**) {
    try {
        SDLApp app("MenuBar - Civilization style", 900, 620, true);
        SDL_Renderer* renderer = app.getRenderer();
        int windowWidth = 0;
        int windowHeight = 0;
        app.getWindowSize(windowWidth, windowHeight);

        GUIManager guiManager(renderer, Viewport{windowWidth, windowHeight});
        guiManager.setTheme(Theme::createDefaultTheme());

        auto bar = std::make_unique<MenuBar>(guiManager, 0, 0, windowWidth);
        bar->setAnchor(Anchor::topBar(0));
        MenuBar* menuBar = bar.get();
        guiManager.addElement(std::move(bar));

        const int barHeight = menuBar->getHeight();

        auto content = std::make_unique<Panel>(guiManager, 0, barHeight,
                                               windowWidth, windowHeight - barHeight);
        content->setAnchor(Anchor::pinned(HAnchor::Stretch, VAnchor::Stretch,
                                          0, barHeight, 0, 0));
        content->setBackgroundColor(ElementState::Normal, SDL_Color{0, 0, 0, 255});
        content->setBorder(ElementState::Normal, SDL_Color{128, 128, 128, 255}, 1);
        Panel* contentPanel = content.get();
        guiManager.addElement(std::move(content));

        std::string statusText = "Ready. Press Alt+F, F10 or use the menu.";
        auto status = std::make_unique<Label>(guiManager, 24, 24, statusText, 20);
        status->setTextColor(ElementState::Normal, SDL_Color{235, 235, 235, 255});
        Label* statusLabel = status.get();
        contentPanel->addChild(std::move(status));

        auto hint = std::make_unique<Label>(guiManager, 24, 64,
            "Alt+F opens File, F10 toggles menu mode, Ctrl+S saves, Ctrl+Q quits.", 14);
        hint->setTextColor(ElementState::Normal, SDL_Color{150, 150, 150, 255});
        contentPanel->addChild(std::move(hint));

        auto refreshStatus = [statusLabel, &statusText]() {
            statusLabel->setText(statusText);
        };

        MenuItem& file = menuBar->addMenu("&File");
        file.addItem("&Save Game", [&statusText, &refreshStatus]() {
            statusText = "Game saved.";
            refreshStatus();
        }).setShortcut("Ctrl+S");
        file.addSeparator();
        file.addItem("&Tax Rate", [&statusText, &refreshStatus]() {
            statusText = "Tax Rate dialog (not implemented in this demo).";
            refreshStatus();
        }).setShortcut("Ctrl+T");
        file.addItem("&Luxuries Rate", [&statusText, &refreshStatus]() {
            statusText = "Luxuries Rate dialog (not implemented).";
            refreshStatus();
        }).setShortcut("Ctrl+L");
        file.addItem("&Find City...", [&statusText, &refreshStatus]() {
            statusText = "Find City...";
            refreshStatus();
        }).setShortcut("Ctrl+F");

        MenuItem& options = file.addSubmenu("&Options");
        auto addCheckItem = [&statusText, &refreshStatus](MenuItem& parent,
                                                         std::string_view text,
                                                         bool initial) {
            MenuItem& item = parent.addItem(text);
            item.setChecked(initial);
            MenuItem* stable = &item;
            const std::string label = menu_text::displayText(text);
            item.action = [&statusText, &refreshStatus, stable, label]() {
                statusText = label + (stable->checked ? ": On" : ": Off");
                refreshStatus();
            };
        };
        addCheckItem(options, "&Instant Advice", true);
        addCheckItem(options, "&Auto Save", true);
        addCheckItem(options, "&End Of Turn", false);
        options.addSeparator();
        addCheckItem(options, "&Animations", true);
        addCheckItem(options, "&Sound", true);
        addCheckItem(options, "&Enemy Moves", true);
        addCheckItem(options, "&Civlopedia Text", true);
        addCheckItem(options, "&Palace", false);

        file.addSeparator();
        file.addItem("&Revolution", [&statusText, &refreshStatus]() {
            statusText = "Revolution!";
            refreshStatus();
        }).setShortcut("Ctrl+R");
        file.addItem("&Retire", [&statusText, &refreshStatus]() {
            statusText = "Retired from the game.";
            refreshStatus();
        });

        bool quit = false;
        file.addItem("&Quit", [&quit]() { quit = true; }).setShortcut("Ctrl+Q");

        MenuItem& edit = menuBar->addMenu("&Edit");
        edit.addItem("&Undo", [&statusText, &refreshStatus]() {
            statusText = "Undo.";
            refreshStatus();
        }).setShortcut("Ctrl+Z");
        edit.addItem("&Redo", nullptr, false).setShortcut("Ctrl+Y");
        edit.addSeparator();
        edit.addItem("Cu&t", nullptr, false).setShortcut("Ctrl+X");
        edit.addItem("&Copy", nullptr, false).setShortcut("Ctrl+C");
        edit.addItem("&Paste", nullptr, false).setShortcut("Ctrl+V");

        MenuItem& orders = menuBar->addMenu("&Orders");
        orders.addItem("&Fortify", [&statusText, &refreshStatus]() {
            statusText = "Unit fortified.";
            refreshStatus();
        });
        orders.addItem("&Sentry", [&statusText, &refreshStatus]() {
            statusText = "Unit on sentry.";
            refreshStatus();
        });
        orders.addItem("&Go To", [&statusText, &refreshStatus]() {
            statusText = "Select destination...";
            refreshStatus();
        });

        MenuItem& advisors = menuBar->addMenu("&Advisors");
        advisors.addItem("&Trade Advisor");
        advisors.addItem("&Military Advisor");
        advisors.addItem("&Science Advisor");

        MenuItem& world = menuBar->addMenu("&World");
        world.addItem("Zoom &In");
        world.addItem("Zoom &Out");
        world.addSeparator();
        MenuItem& tiny = world.addItem("&Tiny").setRadio(true);
        MenuItem& small = world.addItem("&Small").setRadio(false);
        world.addItem("&Large").setRadio(false);
        tiny.action = [&statusText, &refreshStatus]() {
            statusText = "World zoom: Tiny";
            refreshStatus();
        };
        small.action = [&statusText, &refreshStatus]() {
            statusText = "World zoom: Small";
            refreshStatus();
        };

        MenuItem& civlopedia = menuBar->addMenu("&Civlopedia");
        civlopedia.addItem("&Advances");
        civlopedia.addItem("&Units");
        civlopedia.addItem("&City Improvements");

        MenuItem& city = menuBar->addMenu("&City");
        city.addItem("&Change Production");
        city.addItem("&Rename City");

        MenuItem& help = menuBar->addMenu("&Help");
        help.addItem("&About", [&statusText, &refreshStatus]() {
            statusText = "SDL GUI MenuBar demo - Win32-style menus.";
            refreshStatus();
        });

        bool running = true;
        SDL_Event event;
        while (running && !quit) {
            Uint64 frameStart = SDL_GetTicks();
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    running = false;
                    break;
                }
                if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                    guiManager.handleResize(event.window.data1, event.window.data2);
                    continue;
                }
                if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat &&
                    event.key.key == SDLK_ESCAPE &&
                    !menuBar->isMenuOpen() && !menuBar->isMenuMode()) {
                    running = false;
                    break;
                }
                guiManager.processEvent(event);
            }

            guiManager.update();
            guiManager.cleanup();

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            guiManager.render();
            SDL_RenderPresent(renderer);
            app.endFrame(frameStart);
        }
    } catch (const std::runtime_error& error) {
        std::cerr << "Error: " << error.what() << std::endl;
        return 1;
    }
    return 0;
}
