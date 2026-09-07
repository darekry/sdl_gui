/**
 * @file 56_dial_morph.cpp
 * @brief One slider spins a dial needle, another morphs an ArcContainer live.
 *
 * The needle is a thin panel rotated around the arc center via
 * setRotationCenter(); the radius slider calls ArcContainer::setRadius().
 * Clicking an arc button reports its angle.
 */

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "arc_container.hpp"
#include "slider.hpp"
#include "panel.hpp"
#include "label.hpp"
#include "button.hpp"

#include "std.hpp"

int main(int, char**) {
    try {
        SDLApp app("Dial Morph - needle + live arc radius", 800, 600);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{800, 600});
        guiManager.setTheme(Theme::createDefaultTheme());

        constexpr int kCX = 400, kCY = 250;

        auto info = std::make_unique<Label>(guiManager, 10, 10,
            "Top slider spins the needle, bottom slider morphs the arc radius", 16);
        guiManager.addElement(std::move(info));

        auto readout = std::make_unique<Label>(guiManager, 330, 420, "", 18);
        auto readoutRef = guiManager.makeRef(readout.get());
        guiManager.addElement(std::move(readout));

        auto arc = std::make_unique<ArcContainer>(guiManager, kCX, kCY, 150, 0, 360);
        auto* arcPtr = arc.get();

        Style btnStyle;
        btnStyle.backgroundColor = {70, 130, 180, 255};
        btnStyle.borderColor = {255, 255, 255, 255};
        btnStyle.borderWidth = 1;
        btnStyle.borderRadius = 6;
        for (int i = 0; i < 8; ++i) {
            float angle = i * 45.0f;
            auto btn = std::make_unique<Button>(guiManager, 0, 0, 56, 30, std::to_string(static_cast<int>(angle)));
            btn->setStyle(ElementState::Normal, btnStyle);
            btn->setOnClickCallback([angle, readoutRef](GUIElement*) {
                if (readoutRef) readoutRef->setText("arc button at " + std::to_string(static_cast<int>(angle)) + " deg");
            });
            arc->addChildAtAngle(std::move(btn), angle, true);
        }
        guiManager.addElement(std::move(arc));

        // Needle pivoting exactly at the arc center.
        auto needle = std::make_unique<Panel>(guiManager, kCX - 10, kCY - 6, 130, 12);
        needle->setBackgroundColor(ElementState::Normal, {230, 90, 90, 255});
        needle->setRotationCenter(10, 6);
        auto needleRef = guiManager.makeRef(needle.get());
        guiManager.addElement(std::move(needle));

        auto hub = std::make_unique<Panel>(guiManager, kCX - 12, kCY - 12, 24, 24);
        Style hubStyle;
        hubStyle.backgroundColor = {240, 240, 240, 255};
        hubStyle.borderRadius = 12;
        hub->setStyle(ElementState::Normal, hubStyle);
        guiManager.addElement(std::move(hub));

        auto angleSlider = std::make_unique<Slider>(guiManager, 110, 460, 580, 34,
                                                    0, 360, 45, Orientation::Horizontal);
        angleSlider->setTooltip("Needle angle");
        angleSlider->setOnChangeCallback([needleRef, readoutRef](GUIElement* e) {
            auto* s = static_cast<Slider*>(e);
            if (!s) return;
            if (needleRef) needleRef->setRotation(static_cast<double>(s->getValue()));
            if (readoutRef) readoutRef->setText("needle: " + std::to_string(s->getValue()) + " deg");
        });
        guiManager.addElement(std::move(angleSlider));

        auto radiusSlider = std::make_unique<Slider>(guiManager, 110, 510, 580, 34,
                                                     60, 210, 150, Orientation::Horizontal);
        radiusSlider->setTooltip("Arc radius");
        radiusSlider->setOnChangeCallback([arcPtr](GUIElement* e) {
            auto* s = static_cast<Slider*>(e);
            if (s && arcPtr) arcPtr->setRadius(s->getValue());
        });
        guiManager.addElement(std::move(radiusSlider));

        if (needleRef) needleRef->setRotation(45.0);
        if (readoutRef) readoutRef->setText("needle: 45 deg");

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            Uint64 frameStart = SDL_GetTicks();
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                guiManager.processEvent(e);
            }
            guiManager.update();
            guiManager.cleanup();
            SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
            SDL_RenderClear(renderer);
            guiManager.render();
            SDL_RenderPresent(renderer);
            app.endFrame(frameStart);
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
