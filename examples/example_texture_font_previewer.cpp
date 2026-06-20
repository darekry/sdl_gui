#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "panel.hpp"
#include "button.hpp"
#include "combobox.hpp"
#include "text_input.hpp"
#include "label.hpp"
#include "animated_image.hpp"
#include "texture_manager.hpp"
#include "font_manager.hpp"

#include "std.hpp"

int main() {
    try {
        SDLApp app("Texture & Font Previewer", 1200, 800);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager gui(renderer);

        auto main_panel = std::make_unique<Panel>(gui, 50, 50, 1100, 700);

        // ==================== TEXTURE PREVIEW PANEL ====================
        auto tex_panel = std::make_unique<Panel>(gui, 20, 20, 500, 300);
        tex_panel->setBackgroundColor(ElementState::Normal, SDL_Color{230, 230, 240, 255});
        tex_panel->setBorder(ElementState::Normal, SDL_Color{150, 150, 150, 255}, 1);

        tex_panel->addChild(std::make_unique<Label>(gui, 20, 10, "Texture Preview", 16));

        auto tex_preview = std::make_unique<AnimatedImage>(gui, 20, 40, 460, 200);
        tex_preview->setSpriteSheet("assets/button1.png", 1, 1);
        tex_preview->setScaleMode(AnimatedImage::ScaleMode::Center);
        auto texPreviewRef = gui.makeRef(tex_preview.get());
        tex_panel->addChild(std::move(tex_preview));

        tex_panel->addChild(std::make_unique<Label>(gui, 20, 250, "File:", 14));

        auto tex_combo = std::make_unique<ComboBox>(gui, 70, 248, 180, 30);
        tex_combo->addItem("button1.png");
        tex_combo->addItem("button_bg.png");
        tex_combo->addItem("anim.png");
        tex_combo->addItem("explosion.png");
        auto texComboRef = gui.makeRef(tex_combo.get());

        auto updateTexPreview = [texPreviewRef, texComboRef]() {
            if (!texPreviewRef || !texComboRef) return;
            texPreviewRef->setSpriteSheet("assets/" + texComboRef->getSelectedItem(), 1, 1);
        };

        texComboRef->on_selection_changed = [updateTexPreview](int, const std::string&) {
            updateTexPreview();
        };
        tex_panel->addChild(std::move(tex_combo));

        main_panel->addChild(std::move(tex_panel));

        // ==================== FONT PREVIEW PANEL ====================
        auto font_panel = std::make_unique<Panel>(gui, 550, 20, 500, 300);
        font_panel->setBackgroundColor(ElementState::Normal, SDL_Color{230, 240, 230, 255});
        font_panel->setBorder(ElementState::Normal, SDL_Color{150, 150, 150, 255}, 1);

        font_panel->addChild(std::make_unique<Label>(gui, 20, 10, "Font Preview", 16));

        auto font_preview_pane = std::make_unique<Panel>(gui, 20, 40, 460, 230);
        font_preview_pane->setBackgroundColor(ElementState::Normal, SDL_Color{255, 255, 255, 255});
        font_preview_pane->setBorder(ElementState::Normal, SDL_Color{200, 200, 200, 255}, 1);
        auto fontPreviewPaneRef = gui.makeRef(font_preview_pane.get());
        font_panel->addChild(std::move(font_preview_pane));

        main_panel->addChild(std::move(font_panel));

        // ==================== CONTROL PANEL ====================
        auto ctrl_panel = std::make_unique<Panel>(gui, 20, 350, 1030, 200);
        ctrl_panel->setBackgroundColor(ElementState::Normal, SDL_Color{240, 240, 220, 255});
        ctrl_panel->setBorder(ElementState::Normal, SDL_Color{150, 150, 150, 255}, 1);

        ctrl_panel->addChild(std::make_unique<Label>(gui, 20, 15, "Controls", 16));

        ctrl_panel->addChild(std::make_unique<Label>(gui, 20, 55, "Text:", 14));
        auto text_input = std::make_unique<TextInput>(gui, 70, 50, 280, 40);
        text_input->setText(std::string("Hello World!"));
        auto textInputRef = gui.makeRef(text_input.get());
        ctrl_panel->addChild(std::move(text_input));

        ctrl_panel->addChild(std::make_unique<Label>(gui, 380, 55, "Font:", 14));
        auto font_combo = std::make_unique<ComboBox>(gui, 430, 50, 200, 40);
        font_combo->addItem("DejaVuSans.ttf");
        font_combo->addItem("DejaVuSansMono.ttf");
        font_combo->setSelectedIndex(0);
        auto fontComboRef = gui.makeRef(font_combo.get());
        ctrl_panel->addChild(std::move(font_combo));

        ctrl_panel->addChild(std::make_unique<Label>(gui, 660, 55, "Size:", 14));
        auto size_combo = std::make_unique<ComboBox>(gui, 710, 50, 100, 40);
        for (int s : {12, 16, 20, 24, 28, 32, 36, 48}) {
            size_combo->addItem(std::to_string(s));
        }
        size_combo->setSelectedIndex(2);
        auto sizeComboRef = gui.makeRef(size_combo.get());
        ctrl_panel->addChild(std::move(size_combo));

        auto size_info = std::make_unique<Label>(gui, 840, 55, "0x0", 14);
        auto sizeInfoRef = gui.makeRef(size_info.get());
        ctrl_panel->addChild(std::move(size_info));

        auto updateFontPreview = [textInputRef, fontComboRef, sizeComboRef, sizeInfoRef, fontPreviewPaneRef, &gui]() {
            if (!textInputRef || !fontComboRef || !sizeComboRef || !sizeInfoRef || !fontPreviewPaneRef) return;

            std::string text = textInputRef->getText();
            if (text.empty()) text = " ";

            std::string fontName = fontComboRef->getSelectedItem();
            int fontSize = std::stoi(sizeComboRef->getSelectedItem());
            std::string fontPath = "assets/fonts/" + fontName;

            auto& fontMgr = gui.getFontManager();
            auto& texMgr = gui.getTextureManager();

            int tw = 0, th = 0;
            fontMgr.getTextSize(text, fontPath, fontSize, &tw, &th);
            sizeInfoRef->setText(std::to_string(tw) + "x" + std::to_string(th));

            SDL_Color textColor = {0, 0, 0, 255};
            auto textTex = texMgr.createTextureFromText(text, fontPath, fontSize, textColor);
            if (textTex) {
                fontPreviewPaneRef->setTexture(ElementState::Normal, textTex);
            }
        };

        auto refresh_btn = std::make_unique<Button>(gui, 20, 120, 140, 40, "Update Preview");
        refresh_btn->setOnClickCallback([updateFontPreview](GUIElement*) {
            updateFontPreview();
        });
        ctrl_panel->addChild(std::move(refresh_btn));

        textInputRef->setOnTextChanged([updateFontPreview](TextInput*) { updateFontPreview(); });
        fontComboRef->on_selection_changed = [updateFontPreview](int, const std::string&) { updateFontPreview(); };
        sizeComboRef->on_selection_changed = [updateFontPreview](int, const std::string&) { updateFontPreview(); };

        main_panel->addChild(std::move(ctrl_panel));
        gui.addElement(std::move(main_panel));

        updateFontPreview();

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                gui.processEvent(e);
            }

            gui.cleanup();

            SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
            SDL_RenderClear(renderer);
            gui.render();
            SDL_RenderPresent(renderer);
        }

    } catch (const std::exception& e) {
        std::cerr << "Wyjątek: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
