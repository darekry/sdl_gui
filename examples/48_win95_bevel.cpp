#include "anchor.hpp"
#include "button.hpp"
#include "constants.hpp"
#include "gui.hpp"
#include "gui_manager.hpp"
#include "label.hpp"
#include "panel.hpp"
#include "progress_bar.hpp"
#include "sdl_app.hpp"
#include "text_input.hpp"
#include "theme.hpp"

#include "std.hpp"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

std::unique_ptr<Panel> buildWin95Dialog(GUIManager& guiManager) {
    auto dialog = std::make_unique<Panel>(guiManager, 0, 0, 520, 380);
    dialog->setBackgroundColor(ElementState::Normal, constants::kWin95Face);
    dialog->setBevel(ElementState::Normal, BevelType::Raised);
    dialog->setDraggable(true);
    dialog->setAnchor(Anchor::center());

    auto dialogRef = guiManager.makeRef(dialog.get());

    auto titleBar = std::make_unique<Panel>(guiManager, 2, 2, 516, 26);
    titleBar->setBackgroundColor(ElementState::Normal, {0, 0, 128, 255});
    titleBar->setAnchor(Anchor::topBar(2, 2, 2));

    auto titleText = std::make_unique<Label>(guiManager, 6, 5, "Bevel.exe", 14);
    Style titleTextStyle;
    titleTextStyle.textColor = {255, 255, 255, 255};
    titleText->setStyle(ElementState::Normal, titleTextStyle);
    titleBar->addChild(std::move(titleText));

    auto closeButton = std::make_unique<Button>(guiManager, 0, 0, 20, 16, "x");
    closeButton->setAnchor(Anchor::topRight(5));
    closeButton->setOnClickCallback([dialogRef](GUIElement*) {
        if (dialogRef) {
            dialogRef->markForDeletion();
        }
    });
    titleBar->addChild(std::move(closeButton));

    dialog->addChild(std::move(titleBar));

    auto raisedLabel = std::make_unique<Label>(guiManager, 20, 48, "Raised:", 14);
    dialog->addChild(std::move(raisedLabel));
    auto raisedPanel = std::make_unique<Panel>(guiManager, 84, 40, 140, 34);
    raisedPanel->setBackgroundColor(ElementState::Normal, constants::kWin95Face);
    raisedPanel->setBevel(ElementState::Normal, BevelType::Raised);
    dialog->addChild(std::move(raisedPanel));

    auto sunkenLabel = std::make_unique<Label>(guiManager, 20, 94, "Sunken:", 14);
    dialog->addChild(std::move(sunkenLabel));
    auto sunkenPanel = std::make_unique<Panel>(guiManager, 84, 86, 140, 34);
    sunkenPanel->setBackgroundColor(ElementState::Normal, constants::kWin95Face);
    sunkenPanel->setBevel(ElementState::Normal, BevelType::Sunken);
    dialog->addChild(std::move(sunkenPanel));

    auto inputLabel = std::make_unique<Label>(guiManager, 20, 140, "Nazwa pliku:", 14);
    dialog->addChild(std::move(inputLabel));
    auto textInput = std::make_unique<TextInput>(guiManager, 20, 162, 480, 30);
    textInput->setText("C:\\WINDOWS\\SYSTEM32");
    textInput->setAnchor(Anchor::horizontalStretch(20, 20));
    dialog->addChild(std::move(textInput));

    auto progress = std::make_unique<ProgressBar>(guiManager, 20, 206, 300, 24);
    progress->setRange(0, 100);
    progress->setValue(65.0f);
    dialog->addChild(std::move(progress));

    auto okButton = std::make_unique<Button>(guiManager, 0, 0, 76, 28, "OK");
    okButton->setAnchor(Anchor::bottomRightAt(12, 34));
    dialog->addChild(std::move(okButton));

    auto cancelButton = std::make_unique<Button>(guiManager, 0, 0, 76, 28, "Anuluj");
    cancelButton->setAnchor(Anchor::bottomRightAt(100, 34));
    dialog->addChild(std::move(cancelButton));

    auto statusBar = std::make_unique<Panel>(guiManager, 2, 354, 516, 24);
    statusBar->setBackgroundColor(ElementState::Normal, constants::kWin95Face);
    statusBar->setBevel(ElementState::Normal, BevelType::Sunken);
    statusBar->setAnchor(Anchor::bottomBar(2, 2, 2));
    auto statusText = std::make_unique<Label>(guiManager, 6, 4, "Gotowe", 13);
    statusBar->addChild(std::move(statusText));
    dialog->addChild(std::move(statusBar));

    return dialog;
}

int main(int, char**) {
    try {
        SDLApp app("Windows 95 Bevel Demo", SCREEN_WIDTH, SCREEN_HEIGHT, true);
        SDL_Renderer* renderer = app.getRenderer();
        int windowWidth = SCREEN_WIDTH;
        int windowHeight = SCREEN_HEIGHT;
        app.getWindowSize(windowWidth, windowHeight);
        GUIManager guiManager(renderer, Viewport{windowWidth, windowHeight});

        guiManager.setTheme(Theme::createWindows95Theme());

        auto hintLabel = std::make_unique<Label>(guiManager, 16, 8, "Przeciagnij okno za tlo, kliknij przyciski — faza odwraca sie w stanie Pressed", 14);
        hintLabel->setAnchor(Anchor::at(16, 8));
        Style hintStyle;
        hintStyle.textColor = {255, 255, 255, 255};
        hintLabel->setStyle(ElementState::Normal, hintStyle);
        guiManager.addElement(std::move(hintLabel));

        auto showButton = std::make_unique<Button>(guiManager, 16, 34, 140, 28, "Pokaż okno");
        showButton->setAnchor(Anchor::at(16, 34));
        showButton->setOnClickCallback([&guiManager](GUIElement*) {
            guiManager.addElement(buildWin95Dialog(guiManager));
        });
        guiManager.addElement(std::move(showButton));

        guiManager.addElement(buildWin95Dialog(guiManager));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                switch (e.type) {
                    case SDL_EVENT_QUIT:
                        quit = true;
                        break;
                    case SDL_EVENT_WINDOW_RESIZED:
                        guiManager.handleResize(e.window.data1, e.window.data2);
                        break;
                    default:
                        guiManager.processEvent(e);
                        break;
                }
            }

            guiManager.update();
            guiManager.cleanup();

            SDL_SetRenderDrawColor(renderer, 0, 128, 128, 255);
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
