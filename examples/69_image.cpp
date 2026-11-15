/**
 * @file 69_image.cpp
 * @brief Image: thin wrapper over a TextureManager-cached SDL texture.
 *
 * Shows the four scale modes and the nearest-neighbour sampling switch for
 * pixel art. The texture is loaded once and shared through TextureManager.
 */

#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "button.hpp"
#include "label.hpp"
#include "slider.hpp"
#include "image.hpp"

#include "std.hpp"

static std::string modeName(Image::ScaleMode mode) {
    switch (mode) {
        case Image::ScaleMode::Fit:     return "Fit";
        case Image::ScaleMode::Stretch: return "Stretch";
        case Image::ScaleMode::Center:  return "Center";
        case Image::ScaleMode::None:    return "None";
    }
    return "?";
}

int main(int, char**) {
    try {
        SDLApp app("Image - SDL texture wrapper", 820, 560);
        SDL_Renderer* renderer = app.getRenderer();
        int ww = 0, wh = 0;
        app.getWindowSize(ww, wh);
        GUIManager guiManager(renderer, Viewport{ww, wh});
        guiManager.setTheme(Theme::createDefaultTheme());

        auto image = std::make_unique<Image>(guiManager, 40, 50, 360, 180);
        auto imgRef = guiManager.makeRef(image.get());
        image->setImage("assets/button1.png");
        image->setBackgroundColor(ElementState::Normal, {35, 38, 48, 255});
        image->setBorder(ElementState::Normal, {90, 96, 110, 255}, 1);
        guiManager.addElement(std::move(image));

        auto info = std::make_unique<Label>(guiManager, 40, 245, "", 16);
        auto infoRef = guiManager.makeRef(info.get());
        guiManager.addElement(std::move(info));

        auto refresh = [imgRef, infoRef]() {
            if (!imgRef) return;
            const auto [iw, ih] = imgRef->getImageSize();
            if (infoRef) {
                infoRef->setText("texture " + std::to_string(iw) + "x" + std::to_string(ih) +
                                 " | element " + std::to_string(imgRef->getWidth()) + "x" +
                                 std::to_string(imgRef->getHeight()) + " | mode " +
                                 modeName(imgRef->getScaleMode()) +
                                 (imgRef->isNearestScaling() ? " | nearest" : ""));
            }
        };

        const Image::ScaleMode modes[] = {Image::ScaleMode::Fit, Image::ScaleMode::Stretch,
                                          Image::ScaleMode::Center, Image::ScaleMode::None};
        for (int i = 0; i < 4; ++i) {
            const Image::ScaleMode mode = modes[i];
            auto btn = std::make_unique<Button>(guiManager, 40 + i * 120, 280, 110, 34,
                                                modeName(mode));
            btn->setOnClickCallback([imgRef, refresh, mode](GUIElement*) {
                if (imgRef) {
                    imgRef->setScaleMode(mode);
                    refresh();
                }
            });
            guiManager.addElement(std::move(btn));
        }

        auto width = std::make_unique<Slider>(guiManager, 40, 340, 360, 20, 80, 480, 360,
                                              Orientation::Horizontal);
        width->setOnChangeCallback([imgRef, refresh](GUIElement* e) {
            auto* slider = static_cast<Slider*>(e);
            if (imgRef && slider) {
                imgRef->setSize(slider->getValue(), 180);
                refresh();
            }
        });
        guiManager.addElement(std::move(width));

        auto nearest = std::make_unique<Button>(guiManager, 40, 390, 180, 32, "Nearest: off");
        nearest->setOnClickCallback([imgRef, refresh, on = false](GUIElement* e) mutable {
            on = !on;
            if (imgRef) {
                imgRef->setNearestScaling(on);
                if (auto* btn = static_cast<Button*>(e)) {
                    btn->setText(on ? "Nearest: on" : "Nearest: off");
                }
                refresh();
            }
        });
        guiManager.addElement(std::move(nearest));

        refresh();

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
            SDL_SetRenderDrawColor(renderer, 24, 26, 33, 255);
            SDL_RenderClear(renderer);
            guiManager.render();
            SDL_RenderPresent(renderer);
            app.endFrame(frameStart);
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
