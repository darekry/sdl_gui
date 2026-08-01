# Wzorzec kodu używanego w przykładach dokumentacji

Minimalny, działający szkielet aplikacji (do adaptacji w dokumentach):

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        // Opcja A: SDLApp + GUIManager (osobno)
        SDLApp app("Tytuł", 800, 600);
        GUIManager manager(app.getRenderer());
        manager.setTheme(ThemePresets::createDarkTheme());  // KONIECZNE
        manager.setWindowSize(800, 600);                    // dla anchorów

        // ... tworzenie widgetów ...

        // Opcja B: GUIContext (wszystko w jednym)
        // GUIContext ctx("Tytuł", 800, 600);
        // GUIManager& manager = ctx.getGUIManager();

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                manager.processEvent(e);    // 1. zdarzenia
            }
            manager.update();               // 2. timery, animacje, tooltipy
            manager.cleanup();              // 3. usuwanie elementów
            SDL_SetRenderDrawColor(app.getRenderer(), 40, 42, 54, 255);
            SDL_RenderClear(app.getRenderer());
            manager.render();               // 4. rysowanie
            SDL_RenderPresent(app.getRenderer());
        }
    } catch (const std::runtime_error& ex) {
        std::fprintf(stderr, "%s\n", ex.what());
        return 1;
    }
    return 0;
}
```

Krytyczna kolejność: `processEvent` → `update` → `cleanup` → `render`.
Pominięcie `update()` psuje tooltipy; pominięcie `cleanup()` wycieka elementy
z `markForDeletion()`.
