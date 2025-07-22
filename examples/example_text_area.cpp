#include "SDL_clipboard.h"
#include "SDL_stdinc.h"
#include "gui_manager.hpp"
#include "text_area.hpp"
#include "helpers/sdl_app.hpp"
import std.compat;

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    try {
        SDLApp app("TextArea Example", 800, 600);
        GUIManager guiManager(app.getRenderer());

        // Ustawienie domyślnej czcionki dla FontManagera, aby inne elementy GUI też mogły z niej korzystać
        try {
            guiManager.getFontManager().loadDefaultFont("assets/fonts/font.ttf", 16);
        } catch (const std::runtime_error& e) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot load default font: %s. Using fallback.", e.what());
            // W przypadku braku domyślnej czcionki, można by tu załadować inną lub obsłużyć błąd.
        }

        auto textArea_ptr = std::make_unique<TextArea>(guiManager, 50, 50, 700, 500, "assets/fonts/font.ttf", 24);
        
        // Przekazujemy własność do GUIManager i od razu pobieramy surowy wskaźnik.
        // Jest to bezpieczne, ponieważ cykl życia textArea jest zarządzany przez guiManager
        // i wskaźnik będzie ważny tak długo, jak długo istnieje guiManager.
        TextArea* textArea = static_cast<TextArea*>(guiManager.addElement(std::move(textArea_ptr)));
        
        textArea->setText("");
        textArea->setTextColor({20, 20, 20, 255});
        
        bool quit = false;
        SDL_Event e;
        while (!quit) {

            char* clipboard_text = SDL_GetClipboardText();
            if (clipboard_text) {
                const std::string clipboard_content(clipboard_text);
                if (textArea->getText() != clipboard_content) {
                    textArea->setText(clipboard_content);
                }
                SDL_free(clipboard_text);
            }

            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                }
                guiManager.processEvent(e);
            }

            // Aktualizacja logiki (jeśli jest)

            // Renderowanie
            SDL_SetRenderDrawColor(app.getRenderer(), 220, 220, 220, 255);
            SDL_RenderClear(app.getRenderer());

            guiManager.render();
            guiManager.cleanup();

            SDL_RenderPresent(app.getRenderer());





        }

    } catch (const std::runtime_error& e) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error: %s", e.what());
        return 1;
    }
    return 0;
}