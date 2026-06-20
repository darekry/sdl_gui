#include "gui_manager.hpp"
#include "sdl_app.hpp"
#include "button.hpp"
#include "label.hpp"

#include "../output/embedded_assets.hpp"

#include "std.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 400;

static void registerEmbeddedAssets(GUIManager& guiManager) {
    auto& tm = guiManager.getTextureManager();
    auto& fm = guiManager.getFontManager();

    for (const auto & a : g_embeddedAssets) {
         if (a.fontSize < 0) {
            LOG_INFO("Embedded", "Registering texture: %s (%zu bytes)", a.name, a.size);
            tm.loadTextureFromMemory(a.data, a.size, a.name);
        } else {
            LOG_INFO("Embedded", "Registering font: %s size=%d (%zu bytes)", a.name, a.fontSize, a.size);
            fm.loadFontFromMemory(a.data, a.size, a.fontSize, a.name);
        }
    }
}

int main(int, char**) {
    try {
        SDLApp app("Embedded Assets Example", SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager guiManager(renderer);

        registerEmbeddedAssets(guiManager);

        // Przycisk 1: tekstura z embeddowanych assetow — ladowana przez sciezke jak z pliku
        auto btn1 = std::make_unique<Button>(guiManager, 50, 50, 200, 50, "Embedded Texture");

        SharedTexture embeddedTex = guiManager.getTextureManager().loadTexture("assets/button1.png");

        Style texNormal;
        texNormal.texture = embeddedTex;
        texNormal.textColor = {255, 255, 255, 255};
        texNormal.borderRadius = 4;
        btn1->setStyle(ElementState::Normal, texNormal);

        Style texHover;
        texHover.texture = embeddedTex;
        texHover.textColor = {255, 255, 100, 255};
        texHover.borderRadius = 8;
        btn1->setStyle(ElementState::Hover, texHover);

        btn1->setOnClickCallback([](GUIElement*) {
            LOG_INFO("Embedded", "Texture button clicked!");
        });
        guiManager.addElement(std::move(btn1));

        // Przycisk 2: embedded tekstura tla + embedded font (domyslny "assets/fonts/font.ttf")
        auto btn2 = std::make_unique<Button>(guiManager, 300, 50, 200, 50, "Embedded Font");

        SharedTexture bgTex = guiManager.getTextureManager().loadTexture("assets/button_bg.png");

        Style btn2Normal;
        btn2Normal.texture = bgTex;
        btn2Normal.textColor = {0, 0, 0, 255};
        btn2Normal.borderRadius = 2;
        btn2Normal.fontSize = 18;
        btn2->setStyle(ElementState::Normal, btn2Normal);

        btn2->setOnClickCallback([](GUIElement*) {
            LOG_INFO("Embedded", "Font button clicked!");
        });
        guiManager.addElement(std::move(btn2));

        // Etykieta: statystyki bezposrednio z g_embeddedAssets[]
        std::string infoText = "Embedded assets: " + std::to_string(g_embeddedAssetCount);
        auto infoLabel = std::make_unique<Label>(guiManager, 50, 150, infoText, 18);
        infoLabel->setTextColor(ElementState::Normal, {200, 200, 200, 255});
        guiManager.addElement(std::move(infoLabel));

        // Etykieta z nazwami i rozmiarami embeddowanych assetow
        std::string directText;
        for (size_t i = 0; i < g_embeddedAssetCount; i++) {
            auto& a = g_embeddedAssets[i];
            directText += a.name;
            directText += " (" + std::to_string(a.size) + " B";
            if (a.fontSize >= 0) directText += ", font=" + std::to_string(a.fontSize);
            directText += ")";
            if (i < g_embeddedAssetCount - 1) directText += "  |  ";
        }
        auto directLabel = std::make_unique<Label>(guiManager, 50, 200, directText, 12);
        directLabel->setTextColor(ElementState::Normal, {150, 150, 150, 255});
        guiManager.addElement(std::move(directLabel));

        // Dostep do surowych danych przez pointer (bezposrednio z ELFa)
        std::string pointerInfo;
        for (size_t i = 0; i < g_embeddedAssetCount; i++) {
            auto& a = g_embeddedAssets[i];
            pointerInfo += std::to_string(reinterpret_cast<uintptr_t>(a.data));
            if (i < g_embeddedAssetCount - 1) pointerInfo += " | ";
        }
        auto ptrLabel = std::make_unique<Label>(guiManager, 50, 240, "Data pointers: " + pointerInfo, 12);
        ptrLabel->setTextColor(ElementState::Normal, {120, 180, 120, 255});
        guiManager.addElement(std::move(ptrLabel));

        auto helpLabel = std::make_unique<Label>(guiManager, 50, 300,
            "ld -r -b binary -> .rodata section. No filesystem reads. Access via _binary_* symbols.", 12);
        helpLabel->setTextColor(ElementState::Normal, {100, 160, 100, 255});
        guiManager.addElement(std::move(helpLabel));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) {
                    quit = true;
                }
                guiManager.processEvent(e);
            }

            guiManager.update();
            guiManager.cleanup();

            SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
            SDL_RenderClear(renderer);

            guiManager.render();

            SDL_RenderPresent(renderer);
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
