#include "gui_manager.hpp"
#include "sdl_app.hpp"
#include "panel.hpp"
#include "slider.hpp"
#include "label.hpp"
#include "style.hpp"

import std.compat;

int main(int, char**) {
    try {
        // Okno i GUI
        SDLApp app("ScrollView Prototype (Panel + Slider)", 900, 620);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager gui(renderer);

        // Parametry layoutu
        const int containerX = 80;
        const int containerY = 60;
        const int containerW = 520;
        const int containerH = 420;

        const int sliderW   = 20;          // ~20 px jak w wymaganiach
        const int viewportW = containerW - sliderW;
        const int viewportH = containerH;
        const int viewportX = 0;
        const int viewportY = 0;

        // Zawartość: długi panel (będzie przewijany w pionie)
        const int itemCount = 30;
        const int itemH = 28;
        const int itemSpacing = 6;
        const int contentPadding = 8;
        const int contentW = viewportW;
        const int contentH = contentPadding * 2 + itemCount * (itemH + itemSpacing);

        // Suwak: zakres [0, contentH - viewportH]
        const int maxOffset = std::max(0, contentH - viewportH);
        const int minOffset = 0;

        // Kontener najwyższego poziomu (dla przejrzystości; nie wymaga clip)
        auto container = std::make_unique<Panel>(gui, containerX, containerY, containerW, containerH);
        {
            Style s;
            s.backgroundColor = {.r=235, .g=235, .b=235, .a=255};
            s.borderColor ={.r=100, .g=100, .b=100, .a=255};
            s.borderWidth = 1;
            container->setStyle(ElementState::Normal, s);
        }
        Panel* container_p = container.get();

        // Viewport: obcina potomków (clip children)
        auto viewport = std::make_unique<Panel>(gui, viewportX, viewportY, viewportW, viewportH);
        {
            Style s;
            s.backgroundColor = {.r=250, .g=250, .b=255, .a=255};
            s.borderColor = {.r=120, .g=160, .b=220, .a=255};
            s.borderWidth = 1;
            viewport->setStyle(ElementState::Normal, s);
        }
        viewport->setClipChildren(true); // gwarancja, choć domyślnie włączone
        Panel* viewport_p = viewport.get();

        // Content: większy panel przewijany w pionie
        auto content = std::make_unique<Panel>(gui, 0, 0, contentW, contentH);
        {
            Style s;
            s.backgroundColor = {.r=245, .g=245, .b=245, .a=255};
            s.borderColor = {.r=180, .g=180, .b=180, .a=255};
            s.borderWidth = 1;
            content->setStyle(ElementState::Normal, s);
        }
        Panel* content_p = content.get();

        // Wypełnienie zawartości wieloma wierszami (etykiety)
        {
            int y = contentPadding;
            for (int i = 0; i < itemCount; ++i) {
                std::string text = "Wiersz " + std::to_string(i + 1);
                auto rowLabel = std::make_unique<Label>(gui, 12, y, text, 16);
                // Kolor tekstu dla czytelności
                rowLabel->setTextColor(ElementState::Normal, SDL_Color{.r=25, .g=25, .b=25, .a=255});
                content_p->addChild(std::move(rowLabel));
                y += itemH + itemSpacing;
            }
        }

        // Slider pionowy po prawej stronie
        auto slider = std::make_unique<Slider>(gui, viewportW, 0, sliderW, containerH, minOffset, maxOffset, /*initial*/0, Orientation::Vertical);
        {
            // Delikatny styl tła toru uzyskamy z tła panelu Slidera
            Style s;
            s.backgroundColor = SDL_Color{.r=230, .g=230, .b=230, .a=255};
            s.borderColor = SDL_Color{.r=150, .g=150, .b=150, .a=255};
            s.borderWidth = 1;
            slider->setStyle(ElementState::Normal, s);
        }
        Slider* slider_p = slider.get();

        // Podpięcie drzewa
        viewport_p->addChild(std::move(content));
        container_p->addChild(std::move(viewport));
        container_p->addChild(std::move(slider));
        gui.addElement(std::move(container));

        // Callback suwaka: ustaw przesunięcie zawartości (y = -value)
        slider_p->setOnChangeCallback([content_p](GUIElement* self) {
            auto* s = static_cast<Slider*>(self);
            if (!s) return;
            const int value = s->getValue();
            // X bez zmian, Y ujemny (scroll w górę)
            content_p->setPosition(0, -value);
        });

        // Pętla zdarzeń
        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                }

                // Obsługa kółka myszy nad viewportem (opcjonalnie)
                if (e.type == SDL_MOUSEWHEEL && viewport_p) {
                    int mx, my;
                    SDL_GetMouseState(&mx, &my);
                    if (viewport_p->contains(mx, my)) {
                        // Scroll w górę: y>0, w dół: y<0
                        const int step = 80; // 80 px / klik jak w profilu "Minimalny pion"
                        int delta = (e.wheel.y > 0) ? -step : step;
                        int newVal = std::clamp(slider_p->getValue() + delta, minOffset, maxOffset);
                        slider_p->setValue(newVal); // wywoła callback i przerysuje
                    }
                }

                gui.processEvent(e);
            }

            // Render
            SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
            SDL_RenderClear(renderer);

            gui.render();
            gui.cleanup();

            SDL_RenderPresent(renderer);
            SDL_Delay(16);
        }

    } catch (const std::runtime_error& err) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error: %s", err.what());
        return 1;
    }
    return 0;
}