Szczegółowy opis architektury SDL GUI

Główne komponenty:
- GUIManager
  - Centralny kontroler aplikacji. Inicjalizuje i zarządza cyklem życia kluczowych menedżerów: [`src/font_manager.hpp`](src/font_manager.hpp), [`src/texture_manager.hpp`](src/texture_manager.hpp), [`src/timer_manager.hpp`](src/timer_manager.hpp) i [`src/animation_manager.hpp`](src/animation_manager.hpp).
  - Przechowuje wszystkie elementy GUI najwyższego poziomu w kontenerze `std::vector<std::unique_ptr<GUIElement>>`, przejmując nad nimi własność.
  - Odpowiada za główną pętlę zdarzeń (`processEvent`), renderowania (`render`) i czyszczenia (`cleanup`).
  - Zarządza globalnym motywem ([`src/theme.hpp`](src/theme.hpp)), który dostarcza domyślne style dla wszystkich widgetów.
  - Kluczowe pliki: [`src/gui_manager.hpp`](src/gui_manager.hpp:19), [`src/gui_manager.cpp`](src/gui_manager.cpp:14).

- GUIElement
  - Abstrakcyjna klasa bazowa dla wszystkich widgetów. Definiuje wspólny interfejs i zachowanie, w tym pozycję, rozmiar, widoczność i stan (np. `Normal`, `Hover`, `Pressed`).
  - Implementuje hierarchię rodzic-dziecko, gdzie dzieci są przechowywane jako `std::vector<std::unique_ptr<GUIElement>>`.
  - Cykl życia: elementy mogą być oznaczone do usunięcia (`markForDeletion`), a następnie są usuwane z pamięci podczas fazy `cleanup`.
  - Mechanizmy: obsługa zdarzeń, tooltipy, timery (poprzez `TimerManager`), style i motywy.
  - Kluczowe pliki: [`src/gui.hpp`](src/gui.hpp:19), [`src/gui.cpp`](src/gui.cpp:8).

- TextureManager
  - Odpowiada za ładowanie, tworzenie i cachowanie tekstur.
  - Ładuje obrazy z plików (`loadTexture`) i tworzy tekstury z tekstu (`createTextureFromText`).
  - Przechowuje tekstury w mapie (`std::map<std::string, SharedTexture>`), aby uniknąć duplikatów. Zwraca `SharedTexture` (`std::shared_ptr<SDL_Texture>`), co zapewnia automatyczne zarządzanie pamięcią.
  - Tworzy domyślną teksturę zastępczą na wypadek niepowodzenia ładowania.
  - Kluczowe pliki: [`src/texture_manager.hpp`](src/texture_manager.hpp:15), [`src/texture_manager.cpp`](src/texture_manager.cpp:1).

- FontManager
  - Analogiczny do `TextureManager`, ale dla czcionek. Ładuje i cache'uje czcionki TTF.
  - Kluczem w mapie cache'u jest para (ścieżka pliku, rozmiar czcionki).
  - Zwraca `SharedFont` (`std::shared_ptr<TTF_Font>`).
  - Udostępnia funkcję `getTextSize` do precyzyjnego mierzenia wymiarów tekstu.
  - Kluczowe pliki: [`src/font_manager.hpp`](src/font_manager.hpp:30), [`src/font_manager.cpp`](src/font_manager.cpp:7).

Render flow (szczegółowo):
1.  [`GUIManager::render()`](src/gui_manager.cpp:51) iteruje po wszystkich elementach najwyższego poziomu i wywołuje na nich [`GUIElement::render(renderer)`](src/gui.cpp:127).
2.  [`GUIElement::render()`](src/gui.cpp:133) najpierw sprawdza, czy element jest widoczny. Jeśli tak, oblicza swój prostokąt na ekranie i sprawdza przecięcie z prostokątem przycinania rodzica (`parent_clip_rect`).
3.  Następnie sprawdza, czy element chce być renderowany bezpośrednio (`wantsDirectRender()`).
    -   **Ścieżka bezpośrednia**: Jeśli `true`, ustawia przycinanie (`SDL_RenderSetClipRect`), wywołuje wirtualną metodę `drawDirect(renderer)`, a następnie renderuje swoje dzieci. Ta ścieżka jest przeznaczona dla elementów, które często się zmieniają lub wymagają niestandardowego renderowania.
    -   **Ścieżka buforowana (domyślna)**: Jeśli `false`, sprawdza flagę `m_isDirty`. Jeśli flaga jest ustawiona, wywołuje `renderToCache()`.
4.  [`GUIElement::renderToCache()`](src/gui.cpp:174) tworzy (lub odtwarza, jeśli zmienił się rozmiar) teksturę `m_cachedTexture`. Ustawia tę teksturę jako cel renderowania (`SDL_SetRenderTarget`), czyści ją, a następnie wywołuje wirtualną metodę `draw(renderer)`, która zawiera logikę rysowania specyficzną dla danego widgetu. Na koniec przywraca poprzedni cel renderowania.
5.  Po upewnieniu się, że cache jest aktualny, `GUIElement::render()` kopiuje odpowiedni fragment `m_cachedTexture` na główny renderer za pomocą `SDL_RenderCopy`.
6.  Na koniec, `GUIElement::render()` rekurencyjnie wywołuje `render()` dla wszystkich swoich dzieci, przekazując im odpowiedni prostokąt przycinania.

Zarządzanie zasobami i cache:
- **Zarządzanie pamięcią**: Biblioteka intensywnie wykorzystuje inteligentne wskaźniki do automatyzacji zarządzania pamięcią zasobów SDL. `SharedTexture` i `SharedFont` to aliasy na `std::shared_ptr` z niestandardowymi deleterami (zdefiniowanymi w [`src/sdl_deleters.hpp`](src/sdl_deleters.hpp)), które automatycznie zwalniają zasoby SDL, gdy nie są już potrzebne. Elementy GUI i ich dzieci są zarządzane przez `std::unique_ptr`, co zapewnia, że są one poprawnie niszczone.
- **Cache zasobów**: `TextureManager` i `FontManager` działają jako globalne (w ramach `GUIManager`) pule zasobów. Gdy zasób jest żądany, menedżer najpierw sprawdza swój wewnętrzny cache (mapę). Jeśli zasób jest już załadowany, zwraca istniejący wskaźnik. W przeciwnym razie ładuje go, zapisuje w cache'u i zwraca.
- **Cache renderowania**: Każdy `GUIElement` (który nie używa `drawDirect`) posiada własny lokalny cache w postaci tekstury `m_cachedTexture`. Jest on unieważniany (`m_isDirty = true`) gdy stan lub właściwości elementu się zmieniają. To znacznie optymalizuje wydajność, ponieważ skomplikowane, statyczne elementy są rysowane tylko raz, a następnie ich gotowy obraz jest po prostu kopiowany na ekran.

Ścieżki kluczowych plików źródłowych:
- GUIElement (baza): [`src/gui.hpp`](src/gui.hpp:19), [`src/gui.cpp`](src/gui.cpp:8)
- GUIManager: [`src/gui_manager.hpp`](src/gui_manager.hpp:19), [`src/gui_manager.cpp`](src/gui_manager.cpp:14)
- TextureManager: [`src/texture_manager.hpp`](src/texture_manager.hpp:15), [`src/texture_manager.cpp`](src/texture_manager.cpp:1)
- FontManager: [`src/font_manager.hpp`](src/font_manager.hpp:30), [`src/font_manager.cpp`](src/font_manager.cpp:7)
- Style/Theme: [`src/style.hpp`](src/style.hpp:17), [`src/theme.hpp`](src/theme.hpp:10), [`src/theme.cpp`](src/theme.cpp:1)
- Zarządzanie pamięcią: [`src/sdl_deleters.hpp`](src/sdl_deleters.hpp)