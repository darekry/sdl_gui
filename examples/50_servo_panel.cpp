/**
 * @file 50_servo_panel.cpp
 * @brief Three sliders act as servos: X, Y and rotation of a small panel.
 *
 * The dot lives inside a bounded playfield (child coordinates are relative
 * to the parent), sliders drive setPosition()/setRotation() live, and a
 * readout label shows the current pose.
 */

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "slider.hpp"
#include "panel.hpp"
#include "label.hpp"

#include "std.hpp"

int main(int, char**) {
    try {
        SDLApp app("Servo Panel - sliders drive position", 800, 600);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{800, 600});
        guiManager.setTheme(Theme::createDefaultTheme());

        auto info = std::make_unique<Label>(guiManager, 10, 10,
            "X / Y sliders move the dot, ROT spins it - all live, no animation", 16);
        guiManager.addElement(std::move(info));

        constexpr int kFieldW = 560;
        constexpr int kFieldH = 330;
        constexpr int kDot = 30;

        auto field = std::make_unique<Panel>(guiManager, 110, 50, kFieldW, kFieldH);
        Style fieldStyle;
        fieldStyle.backgroundColor = {30, 32, 44, 255};
        fieldStyle.borderColor = {98, 114, 164, 255};
        fieldStyle.borderWidth = 2;
        fieldStyle.borderRadius = 8;
        field->setStyle(ElementState::Normal, fieldStyle);

        auto dot = std::make_unique<Panel>(guiManager, 100, 80, kDot, kDot);
        dot->setBackgroundColor(ElementState::Normal, {220, 90, 90, 255});
        auto dotRef = guiManager.makeRef(dot.get());
        field->addChild(std::move(dot));
        guiManager.addElement(std::move(field));

        auto readout = std::make_unique<Label>(guiManager, 110, 390, "", 18);
        auto readoutRef = guiManager.makeRef(readout.get());
        guiManager.addElement(std::move(readout));

        auto report = [dotRef, readoutRef]() {
            if (!dotRef || !readoutRef) return;
            readoutRef->setText("x=" + std::to_string(dotRef->getX()) +
                                "  y=" + std::to_string(dotRef->getY()) +
                                "  rot=" + std::to_string(static_cast<int>(dotRef->getRotation())) + " deg");
        };

        auto sliderX = std::make_unique<Slider>(guiManager, 110, 425, kFieldW, 32,
                                                0, kFieldW - kDot, 100, Orientation::Horizontal);
        sliderX->setTooltip("Servo X");
        sliderX->setOnChangeCallback([dotRef, report](GUIElement* e) {
            auto* s = static_cast<Slider*>(e);
            if (s && dotRef) {
                dotRef->setPosition(s->getValue(), dotRef->getY());
                report();
            }
        });
        guiManager.addElement(std::move(sliderX));

        auto sliderRot = std::make_unique<Slider>(guiManager, 110, 470, kFieldW, 32,
                                                  0, 360, 0, Orientation::Horizontal);
        sliderRot->setTooltip("Servo rotation");
        sliderRot->setOnChangeCallback([dotRef, report](GUIElement* e) {
            auto* s = static_cast<Slider*>(e);
            if (s && dotRef) {
                dotRef->setRotation(static_cast<double>(s->getValue()));
                report();
            }
        });
        guiManager.addElement(std::move(sliderRot));

        auto sliderY = std::make_unique<Slider>(guiManager, 690, 50, 36, kFieldH,
                                                0, kFieldH - kDot, 80, Orientation::Vertical);
        sliderY->setTooltip("Servo Y");
        sliderY->setOnChangeCallback([dotRef, report](GUIElement* e) {
            auto* s = static_cast<Slider*>(e);
            if (s && dotRef) {
                dotRef->setPosition(dotRef->getX(), s->getValue());
                report();
            }
        });
        guiManager.addElement(std::move(sliderY));
        report();

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
            SDL_SetRenderDrawColor(renderer, 40, 42, 54, 255);
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
