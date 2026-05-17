# Szczegółowy opis architektury SDL GUI

## Główne komponenty

### GUIManager
Centralny kontroler aplikacji. Inicjalizuje i zarządza cyklem życia menedżerów: [`FontManager`](src/font_manager.hpp), [`TextureManager`](src/texture_manager.hpp), [`TimerManager`](src/timer_manager.hpp), [`AnimationManager`](src/animation_manager.hpp).

- Przechowuje elementy GUI najwyższego poziomu jako `std::vector<std::unique_ptr<GUIElement>>`
- Główna pętla zdarzeń (`processEvent`), renderowania (`render`) i czyszczenia (`cleanup`)
- Zarządza globalnym motywem ([`Theme`](src/theme.hpp))
- **Pliki**: [`src/gui_manager.hpp`](src/gui_manager.hpp), [`src/gui_manager.cpp`](src/gui_manager.cpp)

### GUIElement
Abstrakcyjna klasa bazowa dla wszystkich widgetów.

- Definiuje wspólny interfejs: pozycję, rozmiar, widoczność, stan (`Normal`, `Hover`, `Pressed`, `Disabled`)
- Hierarchia rodzic-dziecko (`std::vector<std::unique_ptr<GUIElement>>`)
- Mechanizmy: obsługa zdarzeń, tooltipy, timery, style i motywy
- **Pliki**: [`src/gui.hpp`](src/gui.hpp), [`src/gui.cpp`](src/gui.cpp)

---

## Widgety

### Kontenery
| Widget | Opis | Pliki |
|--------|------|-------|
| **Panel** | Kontener dla innych elementów, obsługuje tło i ramkę | [`src/panel.hpp`](src/panel.hpp), [`src/panel.cpp`](src/panel.cpp) |
| **TabControl** | Zakładki z przełączanymi panelami | [`src/tab_control.hpp`](src/tab_control.hpp), [`src/tab_control.cpp`](src/tab_control.cpp) |

### Kontrolki wejściowe
| Widget | Opis | Pliki |
|--------|------|-------|
| **Button** | Przycisk z tekstem/ikoną i callback onClick | [`src/button.hpp`](src/button.hpp), [`src/button.cpp`](src/button.cpp) |
| **Checkbox** | Pole wyboru z etykietą | [`src/checkbox.hpp`](src/checkbox.hpp), [`src/checkbox.cpp`](src/checkbox.cpp) |
| **RadioButton** | Opcja w grupie RadioGroup | [`src/radio_button.hpp`](src/radio_button.hpp), [`src/radio_button.cpp`](src/radio_button.cpp) |
| **RadioGroup** | Grupa wykluczających się RadioButton | [`src/radio_group.hpp`](src/radio_group.hpp), [`src/radio_group.cpp`](src/radio_group.cpp) |
| **Slider** | Suwak do wyboru wartości z zakresu | [`src/slider.hpp`](src/slider.hpp), [`src/slider.cpp`](src/slider.cpp) |
| **TextInput** | Jednolinijkowe pole tekstowe | [`src/text_input.hpp`](src/text_input.hpp), [`src/text_input.cpp`](src/text_input.cpp) |
| **TextArea** | Wielolinijkowe pole tekstowe | [`src/text_area.hpp`](src/text_area.hpp), [`src/text_area.cpp`](src/text_area.cpp) |
| **ComboBox** | Lista rozwijana z opcjami | [`src/combobox.hpp`](src/combobox.hpp), [`src/combobox.cpp`](src/combobox.cpp) |

### Wyświetlanie danych
| Widget | Opis | Pliki |
|--------|------|-------|
| **Label** | Etykieta tekstowa | [`src/label.hpp`](src/label.hpp), [`src/label.cpp`](src/label.cpp) |
| **StringGrid** | Siatka danych tekstowych z nagłówkami, sortowaniem i edycją inline | [`src/string_grid.hpp`](src/string_grid.hpp), [`src/string_grid.cpp`](src/string_grid.cpp) |

### Grafika i animacje
| Widget | Opis | Pliki |
|--------|------|-------|
| **AnimatedImage** | Animowany sprite z sekwencją klatek | [`src/animated_image.hpp`](src/animated_image.hpp), [`src/animated_image.cpp`](src/animated_image.cpp) |
| **Canvas** | Powierzchnia do rysowania | [`src/canvas.hpp`](src/canvas.hpp), [`src/canvas.cpp`](src/canvas.cpp) |

### Menu
| Widget | Opis | Pliki |
|--------|------|-------|
| **ContextMenu** | Menu kontekstowe (prawy przycisk) | [`src/context_menu.hpp`](src/context_menu.hpp), [`src/context_menu.cpp`](src/context_menu.cpp) |

---

## Menedżery zasobów

### TextureManager
Ładowanie, tworzenie i cache'owanie tekstur.

- Mapa `std::map<std::string, SharedTexture>` zapobiega duplikatom
- Zwraca `SharedTexture` (`std::shared_ptr<SDL_Texture>`)
- **Pliki**: [`src/texture_manager.hpp`](src/texture_manager.hpp), [`src/texture_manager.cpp`](src/texture_manager.cpp)

### FontManager
Ładowanie i cache'owanie czcionek TTF.

- Klucz cache: `(ścieżka_pliku, rozmiar_czcionki)`
- Zwraca `SharedFont` (`std::shared_ptr<TTF_Font>`)
- Funkcja `getTextSize` do mierzenia wymiarów tekstu
- **Pliki**: [`src/font_manager.hpp`](src/font_manager.hpp), [`src/font_manager.cpp`](src/font_manager.cpp)

### TimerManager
Zarządzanie timerami z callbackami.

- **Pliki**: [`src/timer_manager.hpp`](src/timer_manager.hpp), [`src/timer_manager.cpp`](src/timer_manager.cpp)

### AnimationManager
Zarządzanie animacjami z easing functions.

- **Pliki**: [`src/animation_manager.hpp`](src/animation_manager.hpp), [`src/animation_manager.cpp`](src/animation_manager.cpp)

---

## Parsery layoutów

### JsonParser
Definicja GUI z plików JSON.

- Obsługuje wszystkie widgety i sekcje `resources`, `styles`
- Kolory w formatach RGBA i hex
- **Pliki**: [`src/json_parser.hpp`](src/json_parser.hpp), [`src/json_parser.cpp`](src/json_parser.cpp)

### SGMLParser
Definicja GUI z plików XML/SGML.

- Analogiczny do JsonParser, składnia znacznikowa
- **Pliki**: [`src/sgml_parser.hpp`](src/sgml_parser.hpp), [`src/sgml_parser.cpp`](src/sgml_parser.cpp)

### LayoutParser
Wspólny interfejs dla parserów.

- **Pliki**: [`src/layout_parser.hpp`](src/layout_parser.hpp)

---

## Narzędzia

### Style & Theme
System stylów i motywów dla spójnego wyglądu.

- **Pliki**: [`src/style.hpp`](src/style.hpp), [`src/theme.hpp`](src/theme.hpp), [`src/theme.cpp`](src/theme.cpp)

### Easing
Funkcje easingu dla animacji (header-only).

- **Pliki**: [`src/easing.hpp`](src/easing.hpp)

### Cursor
Zarządzanie kursorem myszy (header-only).

- **Pliki**: [`src/cursor.hpp`](src/cursor.hpp)

### SDL App
Narzędzia inicjalizacji aplikacji SDL (header-only).

- **Pliki**: [`src/sdl_app.hpp`](src/sdl_app.hpp)

### SDL Deleters
Niestandardowe deletery dla inteligentnych wskaźników SDL.

- **Pliki**: [`src/sdl_deleters.hpp`](src/sdl_deleters.hpp)

---

## Render flow

1. `GUIManager::render()` iteruje po elementach i wywołuje `GUIElement::render()`
2. `GUIElement::render()` sprawdza widoczność i przecięcie z `parent_clip_rect`
3. **Ścieżka bezpośrednia** (`wantsDirectRender() = true`):
   - Ustawia przycinanie, wywołuje `drawDirect()`, renderuje dzieci
4. **Ścieżka buforowana** (domyślna):
   - Jeśli `m_isDirty`, wywołuje `renderToCache()` → tworzy teksturę `m_cachedTexture`, wywołuje `draw()`
   - Kopiuje cache na renderer przez `SDL_RenderCopy`
5. Rekursywnie renderuje dzieci z odpowiednim prostokątem przycinania

---

## Zarządzanie pamięcią

- **Elementy GUI**: `std::unique_ptr` z hierarchią rodzic-dziecko
- **Zasoby SDL**: `SharedTexture`, `SharedFont` - aliasy na `std::shared_ptr` z niestandardowymi deleterami ([`src/sdl_deleters.hpp`](src/sdl_deleters.hpp))
- **Cache zasobów**: `TextureManager` i `FontManager` jako pule współdzielone
- **Cache renderowania**: Każdy `GUIElement` ma `m_cachedTexture` unieważniany przez `m_isDirty = true`