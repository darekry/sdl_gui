#include "gui_manager.hpp"
#include "sdl_app.hpp"
#include "scroll_area.hpp"
#include "panel.hpp"
#include "button.hpp"
#include "label.hpp"
#include "checkbox.hpp"
#include "slider.hpp"
#include "style.hpp"

#include "std.hpp"

int main(int, char**) {
    try {
        SDLApp app("ScrollArea Example", 600, 500);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager gui(renderer, Viewport{600, 500});

        auto mainPanel = std::make_unique<Panel>(gui, 10, 10, 580, 480);
        {
            Style s;
            s.backgroundColor = {40, 40, 50, 255};
            s.borderColor = {80, 80, 100, 255};
            s.borderWidth = 2;
            s.borderRadius = 6;
            mainPanel->setStyle(ElementState::Normal, s);
        }
        Panel* mainPanelPtr = mainPanel.get();

        auto scroll = std::make_unique<ScrollArea>(gui, 20, 50, 320, 380);
        {
            Style vs;
            vs.backgroundColor = {60, 60, 75, 255};
            vs.borderColor = {100, 100, 120, 255};
            vs.borderWidth = 1;
            scroll->setStyle(ElementState::Normal, vs);
        }
        scroll->setVerticalScroll(true);
        scroll->setHorizontalScroll(false);
        ScrollArea* scrollPtr = scroll.get();

        // Build content: many items
        auto content = std::make_unique<Panel>(gui, 0, 0, 320, 1200);
        {
            Style cs;
            cs.backgroundColor = {50, 50, 65, 255};
            content->setStyle(ElementState::Normal, cs);
        }
        Panel* contentPtr = content.get();

        for (int i = 0; i < 25; ++i) {
            int y = 10 + i * 46;
            auto row = std::make_unique<Panel>(gui, 10, y, 300, 40);
            {
                Style rs;
                rs.backgroundColor = {70, 70, 90, 255};
                rs.borderColor = {120, 120, 150, 255};
                rs.borderWidth = 1;
                rs.borderRadius = 4;
                row->setStyle(ElementState::Normal, rs);
            }

            auto label = std::make_unique<Label>(gui, 12, 10,
                "Item " + std::to_string(i + 1), 16);
            label->setTextColor(ElementState::Normal, {220, 220, 240, 255});
            row->addChild(std::move(label));

            auto btn = std::make_unique<Button>(gui, 200, 5, 90, 30,
                "Select");
            btn->setOnClickCallback([i](GUIElement*) {
                LOG_INFO("ScrollArea", "Selected item {}", i + 1);
            });
            row->addChild(std::move(btn));

            contentPtr->addChild(std::move(row));
        }

        scrollPtr->setContent(std::move(content));
        scrollPtr->setContentSize(320, 1200);

        // Controls panel (right side)
        auto ctrlPanel = std::make_unique<Panel>(gui, 360, 50, 200, 380);
        {
            Style cs;
            cs.backgroundColor = {45, 45, 55, 255};
            cs.borderColor = {80, 80, 100, 255};
            cs.borderWidth = 1;
            cs.borderRadius = 4;
            ctrlPanel->setStyle(ElementState::Normal, cs);
        }
        Panel* ctrlPtr = ctrlPanel.get();

        auto titleLabel = std::make_unique<Label>(gui, 10, 10,
            "Scroll Controls", 18);
        titleLabel->setTextColor(ElementState::Normal, {200, 200, 220, 255});
        ctrlPtr->addChild(std::move(titleLabel));

        auto hCheck = std::make_unique<Checkbox>(gui, 10, 50, 20, 20);
        auto hCheckLabel = std::make_unique<Label>(gui, 40, 48,
            "Horizontal scroll", 14);
        hCheckLabel->setTextColor(ElementState::Normal, {180, 180, 200, 255});
        hCheck->setChecked(false);
        hCheck->setOnChange([scrollPtr](Checkbox*, bool checked) {
            scrollPtr->setHorizontalScroll(checked);
            if (checked) {
                scrollPtr->setContentSize(600, 1200);
            } else {
                scrollPtr->setContentSize(320, 1200);
            }
        });
        ctrlPtr->addChild(std::move(hCheck));
        ctrlPtr->addChild(std::move(hCheckLabel));

        auto resetBtn = std::make_unique<Button>(gui, 10, 90, 180, 30,
            "Scroll to Top");
        resetBtn->setOnClickCallback([scrollPtr](GUIElement*) {
            scrollPtr->setScrollOffset(0, 0);
        });
        ctrlPtr->addChild(std::move(resetBtn));

        auto bottomBtn = std::make_unique<Button>(gui, 10, 130, 180, 30,
            "Scroll to Bottom");
        bottomBtn->setOnClickCallback([scrollPtr](GUIElement*) {
            scrollPtr->setScrollOffset(0, 9999);
        });
        ctrlPtr->addChild(std::move(bottomBtn));

        auto sliderLabel = std::make_unique<Label>(gui, 10, 180,
            "External slider sync:", 14);
        sliderLabel->setTextColor(ElementState::Normal, {180, 180, 200, 255});
        ctrlPtr->addChild(std::move(sliderLabel));

        auto extSlider = std::make_unique<Slider>(gui, 10, 210, 180, 30,
            0, 100, 0, Orientation::Horizontal);
        extSlider->setOnChangeCallback([scrollPtr](GUIElement* self) {
            auto* s = static_cast<Slider*>(self);
            int maxOff = 1200 - 380;
            int offset = (s->getValue() * maxOff) / 100;
            scrollPtr->setScrollOffset(0, offset);
        });
        ctrlPtr->addChild(std::move(extSlider));

        mainPanelPtr->addChild(std::move(scroll));
        mainPanelPtr->addChild(std::move(ctrlPanel));
        gui.addElement(std::move(mainPanel));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                gui.processEvent(e);
            }

            gui.update();
            gui.cleanup();

            SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
            SDL_RenderClear(renderer);
            gui.render();
            SDL_RenderPresent(renderer);
        }

    } catch (const std::runtime_error& err) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error: %s", err.what());
        return 1;
    }
    return 0;
}
