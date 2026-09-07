/**
 * @file 57_text_scaler.cpp
 * @brief Sliders control label font size, rotation and brightness live.
 *
 * Shows that text is just another style: fontSize/textColor go through
 * setStyle() while the angle goes through setRotation(). All three share
 * one refresh() so partial styles never clobber each other.
 */

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "slider.hpp"
#include "label.hpp"

#include "std.hpp"

int main(int, char**) {
    try {
        SDLApp app("Text Scaler - sliders style the text", 800, 600);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer, Viewport{800, 600});
        guiManager.setTheme(Theme::createDefaultTheme());

        auto info = std::make_unique<Label>(guiManager, 10, 10,
            "Sliders change font size, rotation and brightness of the big label", 16);
        guiManager.addElement(std::move(info));

        auto big = std::make_unique<Label>(guiManager, 120, 150, "Hello SDL GUI!", 32);
        auto bigRef = guiManager.makeRef(big.get());
        guiManager.addElement(std::move(big));

        auto readout = std::make_unique<Label>(guiManager, 120, 330, "", 17);
        auto readoutRef = guiManager.makeRef(readout.get());
        guiManager.addElement(std::move(readout));

        int fontSize = 32;
        int angle = 0;
        int gray = 255;

        std::function<void()> refresh = [bigRef, readoutRef, &fontSize, &angle, &gray]() {
            if (!bigRef || !readoutRef) return;
            Style s;
            s.fontSize = fontSize;
            s.textColor = {static_cast<Uint8>(gray), static_cast<Uint8>(gray),
                           static_cast<Uint8>(gray), 255};
            bigRef->setStyle(ElementState::Normal, s);
            bigRef->setRotation(static_cast<double>(angle));
            readoutRef->setText("size=" + std::to_string(fontSize) +
                                "  rot=" + std::to_string(angle) +
                                "  gray=" + std::to_string(gray));
        };

        auto sizeSlider = std::make_unique<Slider>(guiManager, 120, 380, 560, 32,
                                                   8, 72, 32, Orientation::Horizontal);
        sizeSlider->setTooltip("Font size");
        sizeSlider->setOnChangeCallback([&fontSize, refresh](GUIElement* e) {
            auto* s = static_cast<Slider*>(e);
            if (!s) return;
            fontSize = s->getValue();
            refresh();
        });
        guiManager.addElement(std::move(sizeSlider));

        auto rotSlider = std::make_unique<Slider>(guiManager, 120, 440, 560, 32,
                                                  0, 360, 0, Orientation::Horizontal);
        rotSlider->setTooltip("Text rotation");
        rotSlider->setOnChangeCallback([&angle, refresh](GUIElement* e) {
            auto* s = static_cast<Slider*>(e);
            if (!s) return;
            angle = s->getValue();
            refresh();
        });
        guiManager.addElement(std::move(rotSlider));

        auto graySlider = std::make_unique<Slider>(guiManager, 120, 500, 560, 32,
                                                   40, 255, 255, Orientation::Horizontal);
        graySlider->setTooltip("Text brightness");
        graySlider->setOnChangeCallback([&gray, refresh](GUIElement* e) {
            auto* s = static_cast<Slider*>(e);
            if (!s) return;
            gray = s->getValue();
            refresh();
        });
        guiManager.addElement(std::move(graySlider));
        refresh();

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
