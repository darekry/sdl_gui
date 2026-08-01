# TextInput

Jednolinijkowe pole tekstowe z kursorem, zaznaczaniem i obsługą schowka.
Nadaje się do haseł, nazw, ścieżek i wszelkich krótkich wpisów.

## Przeznaczenie

TextInput dziedziczy po `TextEditable`, więc dziedziczy całe wspólne API:
selekcję (`hasSelection`/`getSelection`/`clearSelection`/`setSelection`),
dostęp do tekstu (`setText`/`getText`) oraz callback `setOnTextChanged`.
Enter nie wstawia nowej linii — wywołuje `setOnEnterPressed` (typowe dla
formularzy). Pole można zablokować przez `setLocked(true)` (tryb tylko do
odczytu). Widget jest domyślnie osiągalny przez Tab i edycja wymaga fokusu
klawiatury.

## Tworzenie

```cpp
TextInput(GUIManager& manager, int x, int y, int w, int h);
```

```cpp
auto input = std::make_unique<TextInput>(manager, 20, 20, 300, 32);
input->setText("Jan Kowalski");
manager.addElement(std::move(input));
```

## Najważniejsze metody

Metody własne (specyficzne dla TextInput):

| Metoda | Opis |
|--------|------|
| `void setOnTextChanged(const std::function<void(TextInput*)>& callback)` | Rejestruje callback zmiany treści (przesłania wersję z `TextEditable` — argument to `TextInput*`) |
| `void setOnEnterPressed(const std::function<void(TextInput*)>& callback)` | Rejestruje callback wywoływany po naciśnięciu Enter |
| `void setLocked(bool locked)` | Blokuje edycję (tryb tylko do odczytu) |
| `bool isLocked() const` | Czy pole jest zablokowane |

Odziedziczone z `TextEditable` (w sygnaturze własnej TextInput **nie ma**
słowa "selection" — to metody wspólne klasy bazowej):

| Metoda | Opis |
|--------|------|
| `virtual void setText(std::string_view text)` | Ustawia treść pola |
| `const std::string& getText() const` | Zwraca aktualną treść |
| `bool hasSelection() const` | Czy istnieje zaznaczenie |
| `std::string getSelection() const` | Zaznaczony fragment |
| `void clearSelection()` | Czyści zaznaczenie |
| `void setSelection(size_t start, size_t end)` | Zaznacza zakres (indeksy w znakach UTF-8) |

## Callbacki / zdarzenia

- `setOnTextChanged(const std::function<void(TextInput*)>& callback)` —
  wywoływany po każdej zmianie treści (pisanie, wklejanie, usuwanie, `setText`).
- `setOnEnterPressed(const std::function<void(TextInput*)>& callback)` —
  wywoływany po naciśnięciu Enter. Enter nie wstawia znaku nowej linii.
- Obsługa klawiatury (przy fokusie): Ctrl+C / Ctrl+V / Ctrl+X (schowek),
  Ctrl+A (zaznacz wszystko), Backspace/Delete, strzałki, Shift + strzałki
  (rozszerzanie zaznaczenia).

## Przykład

Formularz logowania: pole nazwy, pole hasła i przycisk; Enter w polu hasła
wywołuje ten sam handler co kliknięcie przycisku.

```cpp
#include "sdl_gui.hpp"

int main(int, char**) {
    try {
        SDLApp app("Logowanie", 420, 200);
        GUIManager manager(app.getRenderer());
        manager.setTheme(ThemePresets::createDarkTheme());
        manager.setWindowSize(420, 200);

        auto nameInput = std::make_unique<TextInput>(manager, 20, 20, 380, 32);
        nameInput->setText("admin");

        auto passInput = std::make_unique<TextInput>(manager, 20, 62, 380, 32);

        auto status = std::make_unique<Label>(manager, 20, 104, 380, 30, "Podaj dane");
        auto statusRef = manager.makeRef(status.get());
        auto nameRef = manager.makeRef(nameInput.get());

        passInput->setOnEnterPressed([statusRef](TextInput* input) {
            if (statusRef && input) {
                statusRef->setText("Hasło: " + input->getText());
            }
        });

        auto btn = std::make_unique<Button>(manager, 20, 140, 120, 36, "Zaloguj");
        btn->setOnClickCallback([statusRef, nameRef](GUIElement*) {
            if (statusRef && nameRef) statusRef->setText("Użytkownik: " + nameRef->getText());
        });

        manager.addElement(std::move(status));
        manager.addElement(std::move(btn));
        manager.addElement(std::move(passInput));
        manager.addElement(std::move(nameInput));

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

- **Edycja wymaga fokusu**: kliknięcie ustawia fokus (`setKeyboardFocus`),
  po utracie fokusu (Tab, kliknięcie gdzie indziej) pole przestaje przyjmować
  tekst, a kursora nie ma. Fokus wizualnie zaznacza obwódka.
- **Enter nie tworzy nowej linii** — to pole jednolinijkowe; naciśnięcie Enter
  tylko wywołuje `onEnterPressed`.
- **`setLocked(true)`** wyłącza całą interakcję (wpisywanie, zaznaczanie,
  schowek), ale tekst pozostaje widoczny i można go ustawić przez `setText`.
- **Schowek**: wklejanie Ctrl+V działa tylko przy fokusie; tekst wklejany jest
  w pozycji kursora, zastępując ewentualne zaznaczenie.
- Do odczytu treści z poziomu callbacku użyj argumentu (`TextInput*`) lub
  `ElementRef` — widget może zostać usunięty zanim callback zostanie wywołany.
