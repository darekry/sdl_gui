/**
 * @file 68_menu_bar.cpp
 * @brief Classic Win32-style menu bar: mnemonics, submenus, checks, shortcuts.
 *
 * Alt+F / F10 opens the menu bar, Left/Right switch menus, Up/Down navigate,
 * Enter activates, Esc closes one level. Shortcuts (Ctrl+S/Q...) fire while
 * menus are closed. The menu bar sits at (0,0); app content lives in a Panel
 * anchored below it (Anchor::pinned with topMargin = bar height).
 *
 * View > Menu Style switches the bar/popup Style sets at runtime: Classic
 * (Win95), Midnight and Paper. Styles are applied locally to the MenuBar and
 * every MenuPopup (Normal/Hover/Disabled), so the switcher also demonstrates
 * the menu style system.
 */

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "menu_bar.hpp"
#include "menu_item.hpp"
#include "panel.hpp"
#include "label.hpp"

#include "std.hpp"

namespace {

struct MenuPalette {
    std::string_view name;
    Style barNormal;
    Style barHover;
    Style barDisabled;
    Style popupNormal;
    Style popupHover;
    Style popupDisabled;
};

Style makeBarStyle(SDL_Color background, SDL_Color text) {
    Style style;
    style.backgroundColor = background;
    style.textColor = text;
    style.borderWidth = 0;
    style.borderRadius = 0;
    return style;
}

Style makePopupStyle(SDL_Color background, SDL_Color text, SDL_Color border) {
    Style style = makeBarStyle(background, text);
    style.borderColor = border;
    style.borderWidth = 1;
    return style;
}

MenuPalette makeClassicPalette() {
    const SDL_Color face{192, 192, 192, 255};
    const SDL_Color black{0, 0, 0, 255};
    const SDL_Color navy{0, 0, 128, 255};
    const SDL_Color white{255, 255, 255, 255};
    const SDL_Color gray{128, 128, 128, 255};

    MenuPalette palette;
    palette.name = "Classic (Win95)";
    palette.barNormal = makeBarStyle(face, black);
    palette.barHover = makeBarStyle(navy, white);
    palette.barDisabled = makeBarStyle(face, gray);
    palette.popupNormal = makePopupStyle(white, black, gray);
    palette.popupHover = makePopupStyle(navy, white, gray);
    palette.popupDisabled = makePopupStyle(white, gray, gray);
    return palette;
}

MenuPalette makeMidnightPalette() {
    const SDL_Color background{24, 28, 44, 255};
    const SDL_Color text{222, 228, 244, 255};
    const SDL_Color accent{72, 108, 210, 255};
    const SDL_Color white{255, 255, 255, 255};
    const SDL_Color disabled{112, 118, 136, 255};
    const SDL_Color popupBackground{18, 22, 36, 255};

    MenuPalette palette;
    palette.name = "Midnight";
    palette.barNormal = makeBarStyle(background, text);
    palette.barHover = makeBarStyle(accent, white);
    palette.barDisabled = makeBarStyle(background, disabled);
    palette.popupNormal = makePopupStyle(popupBackground, text, accent);
    palette.popupHover = makePopupStyle(accent, white, accent);
    palette.popupDisabled = makePopupStyle(popupBackground, disabled, accent);
    return palette;
}

MenuPalette makePaperPalette() {
    const SDL_Color background{244, 236, 214, 255};
    const SDL_Color text{58, 48, 38, 255};
    const SDL_Color accent{214, 120, 40, 255};
    const SDL_Color white{255, 255, 255, 255};
    const SDL_Color disabled{158, 146, 128, 255};
    const SDL_Color border{176, 150, 110, 255};
    const SDL_Color popupBackground{252, 247, 236, 255};

    MenuPalette palette;
    palette.name = "Paper";
    palette.barNormal = makeBarStyle(background, text);
    palette.barHover = makeBarStyle(accent, white);
    palette.barDisabled = makeBarStyle(background, disabled);
    palette.popupNormal = makePopupStyle(popupBackground, text, border);
    palette.popupHover = makePopupStyle(accent, white, border);
    palette.popupDisabled = makePopupStyle(popupBackground, disabled, border);
    return palette;
}

void applyMenuPalette(MenuBar& bar, const MenuPalette& palette) {
    bar.setStyle(ElementState::Normal, palette.barNormal);
    bar.setStyle(ElementState::Hover, palette.barHover);
    bar.setStyle(ElementState::Disabled, palette.barDisabled);
    bar.layoutChildren();

    std::function<void(MenuItem&)> applyToNode = [&](MenuItem& node) {
        if (node.hasSubmenu()) {
            if (MenuPopup* popup = bar.getPopupFor(&node)) {
                popup->setStyle(ElementState::Normal, palette.popupNormal);
                popup->setStyle(ElementState::Hover, palette.popupHover);
                popup->setStyle(ElementState::Disabled, palette.popupDisabled);
                popup->refresh();
            }
        }
        for (auto& child : node.submenu) {
            applyToNode(*child);
        }
    };
    for (size_t i = 0; i < bar.getMenuCount(); ++i) {
        applyToNode(*bar.getMenu(i));
    }
}

} // namespace

int main(int, char**) {
    try {
        SDLApp app("MenuBar - Civilization style", 900, 620, true);
        SDL_Renderer* renderer = app.getRenderer();
        int windowWidth = 0;
        int windowHeight = 0;
        app.getWindowSize(windowWidth, windowHeight);

        GUIManager guiManager(renderer, Viewport{windowWidth, windowHeight});
        guiManager.setTheme(Theme::createDefaultTheme());

        const MenuPalette classicPalette = makeClassicPalette();
        const MenuPalette midnightPalette = makeMidnightPalette();
        const MenuPalette paperPalette = makePaperPalette();

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
            "Alt+F opens File, F10 toggles menu mode, Ctrl+S saves, Ctrl+Q quits. "
            "View > Menu Style changes the bar/popup style.", 14);
        hint->setTextColor(ElementState::Normal, SDL_Color{150, 150, 150, 255});
        contentPanel->addChild(std::move(hint));

        auto refreshStatus = [statusLabel, &statusText]() {
            statusLabel->setText(statusText);
        };

        auto selectStyle = [&statusText, &refreshStatus, menuBar](const MenuPalette& palette) {
            applyMenuPalette(*menuBar, palette);
            statusText = "Menu style: " + std::string(palette.name);
            refreshStatus();
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

        MenuItem& view = menuBar->addMenu("&View");
        MenuItem& menuStyle = view.addSubmenu("&Menu Style");
        MenuItem& classicStyleItem = menuStyle.addItem("&Classic").setRadio(true);
        MenuItem& midnightStyleItem = menuStyle.addItem("&Midnight").setRadio(false);
        MenuItem& paperStyleItem = menuStyle.addItem("&Paper").setRadio(false);
        classicStyleItem.action = [&selectStyle, &classicPalette]() { selectStyle(classicPalette); };
        midnightStyleItem.action = [&selectStyle, &midnightPalette]() { selectStyle(midnightPalette); };
        paperStyleItem.action = [&selectStyle, &paperPalette]() { selectStyle(paperPalette); };

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

        applyMenuPalette(*menuBar, classicPalette);

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
