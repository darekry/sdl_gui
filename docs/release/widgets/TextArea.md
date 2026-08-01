# TextArea

Wielolinijkowe pole tekstowe z zawijaniem wierszy (word wrap), przewijaniem
kółkiem myszy, zaznaczaniem i obsługą schowka. Do edycji dłuższych tekstów,
notatek i dzienników zdarzeń.

## Przeznaczenie

TextArea jest **osobną klasą** dziedziczącą bezpośrednio po `GUIElement` — nie
dziedziczy po `TextEditable`, dlatego metody selekcji i `setOnTextChanged` ma
**własne duplikaty** o tych samych sygnaturach co w `TextEditable`. Wymaga
podania ścieżki fontu i rozmiaru w konstruktorze (font ładowany przez
`FontManager`). Enter wstawia nową linię, strzałki góra/dół przechodzą między
wierszami, kółko myszy przewija zawartość. Domyślnie włączone jest zawijanie
wierszy (`setWordWrap(true)`).

## Tworzenie

```cpp
TextArea(GUIManager& manager, int x, int y, int w, int h, std::string_view font_path, int font_size);
```

```cpp
auto area = std::make_unique<TextArea>(manager, 20, 20, 600, 300, "assets/fonts/font.ttf", 16);
area->setText("Pierwsza linia\nDruga linia");
manager.addElement(std::move(area));
```

## Najważniejsze metody

| Metoda | Opis |
|--------|------|
| `void setText(std::string_view text)` | Ustawia treść (widok na tekst) |
| `void setText(std::string&& text)` | Ustawia treść (przeniesienie) |
| `void setText(const char* text)` | Ustawia treść (łańcuch C) |
| `const std::string& getText() const` | Zwraca aktualną treść |
| `void setWordWrap(bool enabled)` | Włącza/wyłącza zawijanie wierszy do szerokości pola |
| `bool getWordWrap() const` | Czy zawijanie jest włączone |
| `void setOnTextChanged(const std::function<void(TextArea*)>& callback)` | Rejestruje callback zmiany treści (argument to `TextArea*`) |
| `void setLocked(bool locked)` | Blokuje edycję (tryb tylko do odczytu) |
| `bool isLocked() const` | Czy pole jest zablokowane |
| `bool hasSelection() const` | Czy istnieje zaznaczenie (własna implementacja) |
| `std::string getSelection() const` | Zaznaczony fragment (własna implementacja) |
| `void clearSelection()` | Czyści zaznaczenie (własna implementacja) |
| `void setSelection(size_t start, size_t end)` | Zaznacza zakres — indeksy w znakach, nie bajtach (własna implementacja) |

## Callbacki / zdarzenia

- `setOnTextChanged(const std::function<void(TextArea*)>& callback)` —
  wywoływany po każdej zmianie treści: wpisanie znaku, Enter, wklejenie,
  usunięcie, `setText`. Odbiorca otrzymuje wskaźnik do zmienionego obszaru.
- Obsługa klawiatury: Ctrl+C / Ctrl+V / Ctrl+X (schowek), Ctrl+A (zaznacz
  wszystko), Backspace/Delete (usuwają zaznaczenie w całości), strzałki
  góra/dół (zmiana wiersza), lewo/prawo, Shift + strzałki (rozszerzanie
  zaznaczenia).
- Mysz: kliknięcie ustawia kursor i włącza SDL text input, przeciąganie
  zaznacza, kółko myszy przewija pionowo.

## Przykład

Notatnik z blokadą edycji i przełącznikiem zawijania wierszy; licznik znaków
aktualizowany z `onTextChanged`.

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        SDLApp app("Notatnik", 700, 500);
        GUIManager manager(app.getRenderer());
        manager.setTheme(ThemePresets::createDarkTheme());
        manager.setWindowSize(700, 500);

        auto area = std::make_unique<TextArea>(manager, 20, 20, 660, 400,
                                               "assets/fonts/font.ttf", 16);
        area->setText("Witaj w notatniku!\nZawijanie wierszy jest domyślnie włączone.");

        auto counter = std::make_unique<Label>(manager, 20, 430, 400, 30, "Znaki: 0");
        auto counterRef = manager.makeRef(counter.get());
        auto areaRef = manager.makeRef(area.get());

        area->setOnTextChanged([counterRef](TextArea* ta) {
            if (ta && counterRef) {
                counterRef->setText("Znaki: " + std::to_string(ta->getText().length()));
            }
        });

        auto wrapLabel = std::make_unique<Label>(manager, 50, 467, 180, 24, "Zawijanie wierszy");
        auto wrap = std::make_unique<Checkbox>(manager, 20, 465, 24, 24);
        wrap->setChecked(true);
        wrap->setOnChange([areaRef](Checkbox*, bool checked) {
            if (areaRef) areaRef->setWordWrap(checked);
        });

        auto lockLabel = std::make_unique<Label>(manager, 290, 467, 180, 24, "Tylko do odczytu");
        auto lock = std::make_unique<Checkbox>(manager, 260, 465, 24, 24);
        lock->setOnChange([areaRef](Checkbox*, bool checked) {
            if (areaRef) areaRef->setLocked(checked);
        });

        manager.addElement(std::move(wrapLabel));
        manager.addElement(std::move(lockLabel));
        manager.addElement(std::move(counter));
        manager.addElement(std::move(wrap));
        manager.addElement(std::move(lock));
        manager.addElement(std::move(area));
        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                manager.processEvent(e);
            }
            manager.update();
            manager.cleanup();
            SDL_SetRenderDrawColor(app.getRenderer(), 40, 42, 54, 255);
            SDL_RenderClear(app.getRenderer());
            manager.render();
            SDL_RenderPresent(app.getRenderer());
        }
    } catch (const std::runtime_error& ex) {
        std::fprintf(stderr, "%s\n", ex.what());
        return 1;
    }
    return 0;
}
```

## Uwagi

- **Font jest wymagany w konstruktorze** — `font_path` musi wskazywać istniejący
  plik TTF (np. `"assets/fonts/font.ttf"`), a `font_size` to rozmiar w pikselach.
  Jeśli font się nie załaduje, obszar się nie rysuje.
- **Edytowalność zależy od hovera/kliknięcia, nie od systemu fokusu** — TextArea
  nie korzysta z `GUIManager`-owego fokusu klawiatury (nie jest domyślnie
  osiągalny przez Tab). Po kliknięciu wewnątrz włączany jest SDL text input;
  zdarzenia klawiatury są obsługiwane, gdy kursor myszy znajduje się nad
  obszarem. Kliknięcie poza obszarem wyłącza text input i chowa kursor.
- **Enter wstawia nową linię** (w przeciwieństwie do `TextInput`) — callbacku
  "Enter wciśnięty" nie ma; nowa linia jest tylko zmianą treści.
- **Selekcja jest własnym API**, a nie odziedziczonym — sygnatury są identyczne
  jak w `TextEditable`, ale to osobne metody klasy. `setSelection` liczy indeksy
  w znakach UTF-8, nie bajtach.
- **Kółko myszy przewija** pionowo, gdy hover nad obszarem; przy dłuższym
  tekście sprawdź, czy zawartość mieści się w wysokości pola.
- `setLocked(true)` wyłącza wpisywanie i zaznaczanie, ale tekst pozostaje
  widoczny i można go nadal ustawiać przez `setText`.
