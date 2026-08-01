# TextEditable

Abstrakcyjna klasa bazowa dla edytowalnych pól tekstowych (TextInput, TextArea).
Zawiera wspólną implementację zaznaczania, schowka (clipboard), kursora i
callbacku `onTextChanged`. Nie tworzy się jej samodzielnie — jest punktem
wyjścia dla konkretnych widgetów tekstowych.

## Przeznaczenie

Klasa gromadzi w jednym miejscu całą logikę wspólną dla pól tekstowych:
przechowywanie tekstu (UTF-8), pozycję kursora z mruganiem, zaznaczenie myszą
oraz klawiaturą (Shift + strzałki), operacje schowka (Ctrl+C / Ctrl+V / Ctrl+X,
zaznacz wszystko Ctrl+A) i wywołanie `onTextChanged` przy każdej zmianie treści.
Metody `updateTextOffset()`, `refreshTextTexture()` i `markNeedsUpdate()` są
czysto wirtualne — konkretna implementacja (jednolinijkowa lub wielolinijkowa)
decyduje, jak tekst jest przewijany i renderowany. Użyj gotowych pochodnych:
TextInput (jedna linia) lub TextArea (wiele linii, oddzielna implementacja).

## Tworzenie

```cpp
TextEditable(GUIManager& manager, int x, int y, int w, int h);
```

Klasa jest abstrakcyjna — nie można jej utworzyć. Zamiast tego używa się
pochodnej:

```cpp
auto input = std::make_unique<TextInput>(manager, 10, 10, 300, 32);
manager.addElement(std::move(input));
```

## Najważniejsze metody

| Metoda | Opis |
|--------|------|
| `bool hasSelection() const` | Czy istnieje aktywne zaznaczenie tekstu |
| `std::string getSelection() const` | Zwraca zaznaczony fragment tekstu |
| `void clearSelection()` | Usuwa zaznaczenie (tekst pozostaje nietknięty) |
| `void setSelection(size_t start, size_t end)` | Zaznacza zakres od `start` do `end` (indeksy w znakach UTF-8, nie bajtach) |
| `virtual void setText(std::string_view text)` | Ustawia całą treść pola; nadmiarowy kursor jest przycinany, wywoływany jest `onTextChanged` |
| `const std::string& getText() const` | Zwraca aktualną treść pola |
| `void setOnTextChanged(const std::function<void(TextEditable*)>& callback)` | Rejestruje callback wywoływany przy każdej zmianie treści |

Dodatkowo klasa dziedziczy po `GUIElement` i w konstruktorze sama włącza
`setCanGetKeyboardFocus(true)` — pole jest domyślnie osiągalne przez nawigację
klawiszem Tab i przyjmuje fokus po kliknięciu.

## Callbacki / zdarzenia

- `setOnTextChanged(const std::function<void(TextEditable*)>& callback)` —
  wywoływany po każdej zmianie tekstu (wpisanie znaku, wklejenie, usunięcie,
  wywołanie `setText`). Odbiorca otrzymuje wskaźnik do zmienionego widgetu.
- `onFocusGained()` / `onFocusLost()` — przeładowane z `GUIElement`; przy
  zyskaniu fokusu włączany jest SDL text input (potrzebny do zdarzeń
  `SDL_EVENT_TEXT_INPUT`), przy utracie — wyłączany.

## Przykład

Ponieważ klasa jest abstrakcyjna, przykład demonstruje wspólne API przez
pochodną `TextInput` (jednolinijkową) — te same metody działają w TextArea:

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        SDLApp app("TextEditable — wspólne API", 800, 600);
        GUIManager manager(app.getRenderer());
        manager.setTheme(ThemePresets::createDarkTheme());
        manager.setWindowSize(800, 600);

        auto input = std::make_unique<TextInput>(manager, 20, 20, 300, 32);
        auto status = std::make_unique<Label>(manager, 20, 60, 400, 30, "Brak zaznaczenia");
        auto ref = manager.makeRef(status.get());

        input->setText("Wpisz coś...");
        input->setOnTextChanged([ref](TextEditable* e) {
            if (!e || !ref) return;
            ref->setText(e->hasSelection()
                ? "Zaznaczono: \"" + e->getSelection() + "\""
                : "Tekst: \"" + e->getText() + "\"");
        });

        manager.addElement(std::move(status));
        manager.addElement(std::move(input));

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

- **Nie tworzy się instancji** — klasa ma czysto wirtualne metody
  (`updateTextOffset`, `refreshTextTexture`, `markNeedsUpdate`). Użyj
  `TextInput` lub `TextArea`.
- **Indeksy zaznaczenia liczone są w znakach UTF-8**, nie bajtach — przy
  polskich znakach i emoji `setSelection(0, 3)` obejmuje 3 znaki niezależnie
  od długości bajtowej.
- **Schowek**: Ctrl+C kopiuje, Ctrl+V wkleja (zastępując zaznaczenie), Ctrl+X
  wycina, Ctrl+A zaznacza całość. Działa tylko przy fokusie klawiatury.
- **Zaznaczenie myszą**: kliknięcie ustawia kursor, przeciąganie rozszerza
  zaznaczenie; Shift + strzałki zaznacza klawiaturą.
- W callbackach po `std::move(widget)` nie odwołuj się do surowego wskaźnika —
  użyj `ElementRef` (jak w przykładzie).
