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
        SDLApp app("Run Callback — SDLApp::run() + events", SCREEN_WIDTH, SCREEN_HEIGHT);
        GUIManager guiManager(app.getRenderer(), Viewport{SCREEN_WIDTH, SCREEN_HEIGHT});
        guiManager.setTheme(Theme::createDefaultTheme());
        auto label = std::make_unique<Label>(
            guiManager, SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 40, "No key pressed", 24);
        auto* labelPtr = label.get();
        guiManager.addElement(std::move(label));

        auto button = std::make_unique<Button>(
            guiManager, SCREEN_WIDTH / 2 - 75, SCREEN_HEIGHT / 2 + 10, 150, 50, "Button");
        guiManager.addElement(std::move(button));

        app.run(guiManager, {40, 42, 54, 255}, [labelPtr](const SDL_Event& e) {
            if (e.type == SDL_EVENT_KEY_DOWN) {
                const char* name = SDL_GetKeyName(e.key.key);
                labelPtr->setText(std::string("Key: ") + name);
            }
            if (e.type == SDL_EVENT_KEY_UP) {
                labelPtr->setText("No key pressed");
            }
        });
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
