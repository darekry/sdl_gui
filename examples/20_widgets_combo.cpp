#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "panel.hpp"
#include "button.hpp"
#include "checkbox.hpp"
#include "label.hpp"
#include "slider.hpp"
#include "combobox.hpp"
#include "context_menu.hpp"
#include "theme.hpp"
#include "anchor.hpp"
#include "std.hpp"

const int SW = 800, SH = 600;

int main(int, char**) {
    try {
        SDLApp app("Widgets Combo Demo", SW, SH);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{SW, SH});
        guiManager.setTheme(Theme::createDefaultTheme());
        // Main panel — centered via anchor, dark, rounded corners
        auto panel = std::make_unique<Panel>(guiManager, 0, 0, 520, 370);
        panel->setAnchor(Anchor::center());
        panel->setTooltip("Right-click for context menu");
        panel->setBackgroundColor(ElementState::Normal, {45, 48, 58, 255});
        panel->setBorder(ElementState::Normal, {98, 114, 164, 255}, 2);
        panel->setBorderRadius(ElementState::Normal, 12);

        Style lt;
        lt.textColor = {235, 235, 245, 255};

        // Title
        auto t = std::make_unique<Label>(guiManager, 20, 20, "Widgets Demo", 28);
        t->setStyle(ElementState::Normal, lt);
        panel->addChild(std::move(t));

        // Button with tooltip
        auto btn = std::make_unique<Button>(guiManager, 20, 75, 150, 36, "Click Me");
        btn->setTooltip("Click to trigger a log message");
        btn->setOnClickCallback([](GUIElement*) { LOG_INFO("Demo", "Button clicked!"); });
        panel->addChild(std::move(btn));

        // Checkbox with tooltip + label
        auto chk = std::make_unique<Checkbox>(guiManager, 20, 130, 26, 26);
        chk->setTooltip("Toggle this option on or off");
        chk->setOnChange([](Checkbox*, bool c) { LOG_INFO("Demo", "Checkbox: {}", c); });
        panel->addChild(std::move(chk));
        auto chkL = std::make_unique<Label>(guiManager, 55, 134, "Enable Option", 16);
        chkL->setStyle(ElementState::Normal, lt);
        panel->addChild(std::move(chkL));
        // Slider with tooltip + live value label (updated via ElementRef)
        auto vl = std::make_unique<Label>(guiManager, 345, 188, "50", 16);
        vl->setStyle(ElementState::Normal, lt);
        auto vlRef = guiManager.makeRef(vl.get());
        panel->addChild(std::move(vl));

        auto sld = std::make_unique<Slider>(guiManager, 20, 185, 310, 30,
                                            0, 100, 50, Orientation::Horizontal);
        sld->setTooltip("Drag to change the numeric value");
        sld->setOnChangeCallback([vlRef](GUIElement* elem) {
            auto* s = static_cast<Slider*>(elem);
            if (s && vlRef) vlRef->setText(std::to_string(s->getValue()));
            LOG_INFO("Demo", "Slider: {}", s ? s->getValue() : 0);
        });
        panel->addChild(std::move(sld));

        // ComboBox with tooltip + label
        auto cbl = std::make_unique<Label>(guiManager, 20, 240, "Select item:", 16);
        cbl->setStyle(ElementState::Normal, lt);
        panel->addChild(std::move(cbl));
        auto combo = std::make_unique<ComboBox>(guiManager, 20, 265, 260, 30);
        combo->addItem("Alpha"); combo->addItem("Beta"); combo->addItem("Gamma"); combo->addItem("Delta");
        combo->setSelectedIndex(0);
        combo->setTooltip("Choose an item from the dropdown list");
        combo->on_selection_changed = [](int i, const std::string& item) {
            LOG_INFO("Demo", "ComboBox: {} (index={})", item, i);
        };
        panel->addChild(std::move(combo));

        // Context menu triggered by the Menu button
        auto ctxMenu = std::make_unique<ContextMenu>(guiManager);
        auto ctxRef = guiManager.makeRef(ctxMenu.get());
        ctxRef->addItem("Info",     []() { LOG_INFO("Demo", "ContextMenu: Info"); });
        ctxRef->addItem("Settings", []() { LOG_INFO("Demo", "ContextMenu: Settings"); });
        ctxRef->addSeparator();
        ctxRef->addItem("Quit",    []() { LOG_INFO("Demo", "ContextMenu: Quit"); });

        auto menuBtn = std::make_unique<Button>(guiManager, 20, 315, 110, 36, "Menu");
        menuBtn->setTooltip("Click to open the context menu");
        menuBtn->setOnClickCallback([ctxRef](GUIElement* elem) {
            if (!ctxRef) return;
            auto pos = elem->getAbsolutePosition();
            ctxRef->showAt(pos.x, pos.y + elem->getHeight());
        });
        panel->addChild(std::move(menuBtn));

        guiManager.addElement(std::move(panel));
        guiManager.addElement(std::move(ctxMenu));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                guiManager.processEvent(e);
            }
            guiManager.update(); guiManager.cleanup();
            SDL_SetRenderDrawColor(renderer, 35, 38, 48, 255);
            SDL_RenderClear(renderer);
            guiManager.render();
            SDL_RenderPresent(renderer);
        }
    } catch (const std::runtime_error& e) { std::cerr << "Error: " << e.what() << std::endl; return 1; }
    return 0;
}
