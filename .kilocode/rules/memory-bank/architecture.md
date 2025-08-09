Szczegółowy opis architektury SDL GUI

Główne komponenty:
- GUIManager
  - Kontroler kontekstu aplikacji i punkt wejścia do renderowania/obsługi zdarzeń.
  - Zarządza instancjami: [`src/font_manager.hpp`](src/font_manager.hpp:30), [`src/texture_manager.hpp`](src/texture_manager.hpp:15), [`src/timer_manager.hpp`](src/timer_manager.hpp:18), [`src/animation_manager.hpp`](src/animation_manager.hpp:24) oraz globalnym [`src/theme.hpp`](src/theme.hpp:10).
  - Kluczowe pliki: [`src/gui_manager.hpp`](src/gui_manager.hpp:19), [`src/gui_manager.cpp`](src/gui_manager.cpp:14).

- GUIElement
  - Abstrakcyjna baza dla wszystkich widgetów (pozycja, rozmiar, widoczność, stan).
  - Mechanizmy: hierarchia rodzic-dziecko, markDirty/markForDeletion, tooltipy, timery, style i motywy.
  - Renderowanie:
    - Dwa tryby: bezpośrednie (drawDirect) i buforowane (renderToCache -> m_cachedTexture).
    - Cache tekstury przechowywane jako std::unique_ptr<SDL_Texture, SDL_DestroyTexture> w [`src/gui.hpp`](src/gui.hpp:19) i używane w implementacji w [`src/gui.cpp`](src/gui.cpp:135-251).
  - Kluczowe pliki: [`src/gui.hpp`](src/gui.hpp:19), [`src/gui.cpp`](src/gui.cpp:8).

- TextureManager
  - Odpowiada za ładowanie i cachowanie obrazów oraz tworzenie tekstur z tekstu.
  - Zwraca `SharedTexture` (std::shared_ptr<SDL_Texture>) i przechowuje mapę cache.
  - Zapewnia `createDefaultTexture` wykorzystywane przy inicjalizacji [`GUIManager`](src/gui_manager.cpp:14).
  - Kluczowe pliki: [`src/texture_manager.hpp`](src/texture_manager.hpp:15), [`src/texture_manager.cpp`](src/texture_manager.cpp:1).

- FontManager
  - Ładuje i cache'uje czcionki (`SharedFont`).
  - Udostępnia `getTextSize` do precyzyjnego dopasowania tooltipów i layoutu.
  - Kluczowe pliki: [`src/font_manager.hpp`](src/font_manager.hpp:30), [`src/font_manager.cpp`](src/font_manager.cpp:7).

- AnimationManager
  - Prosty system animacji, operuje na wskazanych właściwościach (int/float) za pomocą std::variant i easing.
  - Aktualizuje aktywne animacje w `update()`.
  - Kluczowy plik: [`src/animation_manager.hpp`](src/animation_manager.hpp:24).

- TimerManager
  - Harmonogram zdarzeń czasowych powiązanych z elementami GUI.
  - Pozwala elementom uruchamiać timery przez `startTimer` i zatrzymywać `stopTimer`.
  - Kluczowe pliki: [`src/timer_manager.hpp`](src/timer_manager.hpp:18), [`src/timer_manager.cpp`](src/timer_manager.cpp:1).

Render flow (skrót):
1. [`GUIManager::render()`](src/gui_manager.cpp:51) iteruje po top-level elementach i wywołuje [`GUIElement::render()`](src/gui.cpp:135).
2. [`GUIElement::render()`](src/gui.cpp:135) sprawdza `wantsDirectRender()`:
   - Jeśli true: przygotowuje clipping i wywołuje `drawDirect(renderer)`, następnie renderuje dzieci bez użycia cache'u.
   - Jeśli false: jeśli `m_isDirty` -> `renderToCache()` (ustawia render target na `m_cachedTexture`), wywołuje `draw(renderer)` i resetuje target.
3. GUIElement renderuje `m_cachedTexture` na główny renderer (`SDL_RenderCopy`) oraz renderuje dzieci z odpowiednim clippingiem.
4. [`GUIElement::cleanup()`](src/gui.cpp:267) i [`GUIManager::cleanup()`](src/gui_manager.cpp:64) usuwają elementy oznaczone do usunięcia oraz aktualizują timery i animacje.

Cache i zarządzanie zasobami:
- `TextureManager` cache'uje tekstury w mapie `std::map<std::string, SharedTexture>` i udostępnia `getTexture/loadTexture/queryTexture`.
- `FontManager` cache'uje czcionki w `std::map<FontKey, SharedFont>`.
- `GUIElement` utrzymuje lokalny cache renderu (`m_cachedTexture`) dostosowany do rozmiaru elementu; przy zmianie rozmiaru tekstura jest rekreowana w [`GUIElement::renderToCache()`](src/gui.cpp:220-251).

Ścieżki kluczowych plików źródłowych:
- GUI: [`src/gui.hpp`](src/gui.hpp:19), [`src/gui.cpp`](src/gui.cpp:8)
- Manager: [`src/gui_manager.hpp`](src/gui_manager.hpp:19), [`src/gui_manager.cpp`](src/gui_manager.cpp:14)
- TextureManager: [`src/texture_manager.hpp`](src/texture_manager.hpp:15), [`src/texture_manager.cpp`](src/texture_manager.cpp:1)
- FontManager: [`src/font_manager.hpp`](src/font_manager.hpp:30), [`src/font_manager.cpp`](src/font_manager.cpp:7)
- Theme/Style: [`src/style.hpp`](src/style.hpp:17), [`src/theme.hpp`](src/theme.hpp:10), [`src/theme.cpp`](src/theme.cpp:1)
- Animation/Timer: [`src/animation_manager.hpp`](src/animation_manager.hpp:24), [`src/timer_manager.hpp`](src/timer_manager.hpp:18), [`src/timer_manager.cpp`](src/timer_manager.cpp:1)

Uwagi dotyczące spójności i decyzje projektowe:
- Lokalny cache `m_cachedTexture` upraszcza renderowanie i zmniejsza koszt rysowania skomplikowanych drzew widgetów, ale wymaga poprawnej obsługi zmian rozmiaru i błędów tworzenia tekstury (obecnie logowane).
- Menedżery zasobów używają `SharedTexture` / `SharedFont` z niestandardowymi deleterami ([`src/sdl_deleters.hpp`](src/sdl_deleters.hpp:1)) aby bezpiecznie zarządzać zasobami SDL.
- Przy dużych projektach można rozważyć refaktoryzację TextureManager/FontManager do oddzielnej biblioteki lub modułu dla lepszej ponownego użycia.