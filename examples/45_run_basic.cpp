#include "panel.hpp"
#include "label.hpp"
#include "button.hpp"
#include "gui_manager.hpp"
#include "theme.hpp"
#include "sdl_app.hpp"
#include "std.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int, char**)
{
    try
    {
        SDLApp app("Run Basic — SDLApp::run()", SCREEN_WIDTH, SCREEN_HEIGHT);
        GUIManager guiManager(app.getRenderer());
        guiManager.setTheme(Theme::createDefaultTheme());
        guiManager.setWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);

        auto countPtr = std::make_shared<int>(0);

        auto label = std::make_unique<Label>(
            guiManager, SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 60, "Clicks: 0", 24);
        auto* labelPtr = label.get();
        guiManager.addElement(std::move(label));

        auto button = std::make_unique<Button>(
            guiManager, SCREEN_WIDTH / 2 - 75, SCREEN_HEIGHT / 2, 150, 50, "Click Me");
        button->setOnClickCallback([countPtr, labelPtr](GUIElement*) {
            (*countPtr)++;
            labelPtr->setText("Clicks: " + std::to_string(*countPtr));
        });
        guiManager.addElement(std::move(button));

        app.run(guiManager, {40, 42, 54, 255});
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
