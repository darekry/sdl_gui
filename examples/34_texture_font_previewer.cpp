#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "panel.hpp"
#include "combobox.hpp"
#include "text_input.hpp"
#include "texture_manager.hpp"
#include "theme.hpp"
#include "std.hpp"

int main() {
    try {
        SDLApp app("Texture & Font Previewer", 1000, 700);
        SDL_Renderer* renderer = app.getRenderer();
        GUIManager gui(renderer, Viewport{1000, 700});
        gui.setTheme(Theme::createDefaultTheme());

        auto mainPanel = std::make_unique<Panel>(gui, 20, 20, 960, 660);

        // Texture preview — loads images via TextureManager, displays on Panel
        auto texPanel = std::make_unique<Panel>(gui, 10, 10, 450, 280);
        auto texView = std::make_unique<Panel>(gui, 10, 35, 430, 180);
        texView->setBackgroundColor(ElementState::Normal, {255, 255, 255, 255});
        auto texViewRef = gui.makeRef(texView.get());
        texPanel->addChild(std::move(texView));

        auto texCombo = std::make_unique<ComboBox>(gui, 10, 235, 200, 30);
        for (auto name : {"button1.png", "button_bg.png", "anim.png", "explosion.png"})
            texCombo->addItem(name);
        auto texComboRef = gui.makeRef(texCombo.get());
        texPanel->addChild(std::move(texCombo));
        mainPanel->addChild(std::move(texPanel));

        // Font preview — renders text to texture via FontManager
        auto fontPanel = std::make_unique<Panel>(gui, 480, 10, 450, 280);
        auto fontView = std::make_unique<Panel>(gui, 10, 35, 430, 235);
        fontView->setBackgroundColor(ElementState::Normal, {255, 255, 255, 255});
        auto fontViewRef = gui.makeRef(fontView.get());
        fontPanel->addChild(std::move(fontView));
        mainPanel->addChild(std::move(fontPanel));

        // Controls — TextInput for custom text, ComboBox for font size
        auto ctrlPanel = std::make_unique<Panel>(gui, 10, 310, 920, 320);
        auto textInput = std::make_unique<TextInput>(gui, 70, 45, 250, 35);
        textInput->setText("Hello World!");
        auto textInputRef = gui.makeRef(textInput.get());
        ctrlPanel->addChild(std::move(textInput));
        auto sizeCombo = std::make_unique<ComboBox>(gui, 440, 45, 100, 35);
        for (int s : {12, 16, 20, 24, 28, 32, 36, 48})
            sizeCombo->addItem(std::to_string(s));
        sizeCombo->setSelectedIndex(2);
        auto sizeComboRef = gui.makeRef(sizeCombo.get());
        ctrlPanel->addChild(std::move(sizeCombo));
        // Update function — loads texture file and renders text
        auto updatePreview = [texViewRef, texComboRef, textInputRef,
                              sizeComboRef, fontViewRef, &gui]() {
            if (!texViewRef || !texComboRef) return;
            auto tex = gui.getTextureManager().loadTexture(
                "assets/" + texComboRef->getSelectedItem());
            texViewRef->setTexture(ElementState::Normal, tex);

            if (!textInputRef || !sizeComboRef || !fontViewRef) return;
            std::string text = textInputRef->getText();
            if (text.empty()) text = " ";
            int fontSize = std::stoi(sizeComboRef->getSelectedItem());
            std::string fontPath = "assets/fonts/DejaVuSans.ttf";

            auto textTex = gui.getTextureManager().createTextureFromText(
                text, fontPath, fontSize, SDL_Color{0, 0, 0, 255});
            if (textTex)
                fontViewRef->setTexture(ElementState::Normal, textTex);
        };

        texComboRef->on_selection_changed = [updatePreview](int, const std::string&) { updatePreview(); };
        textInputRef->setOnTextChanged([updatePreview](TextInput*) { updatePreview(); });
        sizeComboRef->on_selection_changed = [updatePreview](int, const std::string&) { updatePreview(); };

        mainPanel->addChild(std::move(ctrlPanel));
        gui.addElement(std::move(mainPanel));
        updatePreview();

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                gui.processEvent(e);
            }
            gui.update();
            gui.cleanup();
            SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
            SDL_RenderClear(renderer);
            gui.render();
            SDL_RenderPresent(renderer);
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
