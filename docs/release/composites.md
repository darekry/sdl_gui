# Komponenty złożone

Gotowe, wyższej klasy komponenty dialogowe zbudowane z podstawowych widgetów
(Panel, Label, Button, TextInput, StringGrid). Wszystkie trzy klasy są
**overlayami** — `isOverlay()` zwraca `true`, przez co GUIManager renderuje je
nad pozostałymi elementami, a focus i interakcja nie wychodzą poza dialog.
Dialogi nie potrzebują własnego okna SDL — żyją wewnątrz jednego renderera.

## DialogBox

Dialog z tytułem (opcjonalnie), komunikatem i zestawem przycisków akcji.
Uniwersalny komponent — `Confirm` (Tak/Nie), `Alert` (OK) i `Custom`
(dowolna liczba przycisków). Dziedziczy po `Panel`, więc działa na nim cała
baza `GUIElement` (style, anchor, tooltip itd.).

### Tworzenie

Dialogi tworzy się przez **statyczne fabryki**, które zwracają
`std::unique_ptr<DialogBox>` — tak jak zwykłe widgety trzeba je dodać przez
`manager.addElement(std::move(dialog))`. Konstruktor publiczny istnieje, ale
fabryki pokrywają wszystkie praktyczne przypadki.

```cpp
static std::unique_ptr<DialogBox> createConfirm(
    GUIManager& manager,
    std::string_view message,
    std::string_view yesLabel = "Tak",
    std::string_view noLabel = "Nie",
    std::function<void(bool confirmed)> callback = nullptr,
    int width = 400, int height = 150);

static std::unique_ptr<DialogBox> createAlert(
    GUIManager& manager,
    std::string_view message,
    std::string_view okLabel = "OK",
    DialogCallback callback = nullptr,
    int width = 350, int height = 120);

static std::unique_ptr<DialogBox> createCustom(
    GUIManager& manager,
    std::string_view message,
    const std::vector<std::string>& buttonLabels,
    DialogCallback callback = nullptr,
    int width = 400, int height = 150);

static std::unique_ptr<DialogBox> createWithTitle(
    GUIManager& manager,
    std::string_view title,
    std::string_view message,
    const std::vector<std::string>& buttonLabels,
    DialogCallback callback = nullptr,
    int width = 400, int height = 180);
```

- `createConfirm` — dwa przyciski; callback otrzymuje `bool` (`true` = Tak,
  `false` = Nie).
- `createAlert` — jeden przycisk; callback otrzymuje indeks `0`.
- `createCustom` — przyciski z listy `buttonLabels`; callback otrzymuje indeks
  klikniętego przycisku (0-based).
- `createWithTitle` — jak `createCustom`, plus pasek tytułu.

```cpp
auto dialog = DialogBox::createConfirm(manager, "Czy na pewno usunąć plik?",
                                       "Tak", "Nie",
                                       [](bool confirmed) {
                                           if (confirmed) { /* usuwanie */ }
                                       });
manager.addElement(std::move(dialog));
```

### Najważniejsze metody

| Metoda | Opis |
|--------|------|
| `void setMessage(std::string_view message)` | Zmienia treść komunikatu |
| `void setTitle(std::string_view title)` | Ustawia/zmienia tytuł okna |
| `void close()` | Zamyka dialog (oznacza do usunięcia; wymaga `cleanup()` w pętli) |
| `bool isOpen() const` | `true` dopóki dialog nie został zamknięty |
| `DialogType getDialogType() const` | Typ dialogu: `Confirm`, `Alert` lub `Custom` |
| `int getLastClickedButton() const` | Indeks ostatnio klikniętego przycisku, `-1` jeśli brak |
| `bool isOverlay() const` | Zawsze `true` — dialog jest overlayem |

```cpp
enum class DialogType { Confirm, Alert, Custom };
```

### Callbacki / zdarzenia

- **Confirm**: `std::function<void(bool confirmed)>` — `true` = kliknięto
  `yesLabel`, `false` = kliknięto `noLabel`.
- **Alert / Custom**: `std::function<void(int buttonIndex)>` — indeks
  klikniętego przycisku (0-based, wg kolejności w `buttonLabels`).
- Callback jest opcjonalny (`nullptr` dozwolony).
- Po kliknięciu dowolnego przycisku dialog zamyka się sam; wartość indeksu
  można też odczytać później przez `getLastClickedButton()`.

### Uwagi

- W callbacku nie dotykaj obiektu dialogu — po kliknięciu przycisku jest on
  oznaczony do usunięcia. Jeśli potrzebujesz dostępu do innych widgetów,
  użyj `ElementRef` utworzonego przed `std::move()`.
- `close()` nie usuwa elementu natychmiast — zniknie po `manager.cleanup()`.
- Dialog jest centrowany w oknie automatycznie (nie podajesz pozycji w
  fabryce).

## MessageBox

Statyczna klasa pomocnicza do szybkich komunikatów — jedna linijka kodu bez
tworzenia i dodawania `DialogBox`. Fabryki **same dodają** element do managera
i zwracają surowy `GUIElement*` (np. do późniejszej manipulacji stylem).
Wymiary okna są dobierane automatycznie do długości tekstu.

### Metody statyczne

```cpp
static GUIElement* showInfo(
    GUIManager& manager,
    std::string_view message,
    std::function<void()> callback = nullptr);

static GUIElement* showError(
    GUIManager& manager,
    std::string_view message,
    std::function<void()> callback = nullptr);

static GUIElement* showWarning(
    GUIManager& manager,
    std::string_view message,
    std::function<void()> callback = nullptr);

static GUIElement* showQuestion(
    GUIManager& manager,
    std::string_view message,
    std::function<void()> onYes,
    std::function<void()> onNo = nullptr);

static GUIElement* showCustom(
    GUIManager& manager,
    std::string_view title,
    std::string_view message,
    std::string_view buttonText = "OK",
    IconType icon = IconType::None,
    std::function<void()> callback = nullptr);
```

```cpp
enum class IconType { None, Info, Warning, Error, Question };
```

### Callbacki / zdarzenia

- `showInfo` / `showError` / `showWarning` — jeden przycisk (OK); callback
  wywoływany po kliknięciu.
- `showQuestion` — dwa przyciski: `onYes` (Tak) i opcjonalny `onNo` (Nie).
- `showCustom` — własny tytuł, treść i tekst przycisku; `IconType` jest
  parametrem zarezerwowanym na przyszłość (ikony nie są jeszcze rysowane).

### Przykład

```cpp
MessageBox::showInfo(manager, "Plik został zapisany.",
                     []() { /* po kliknięciu OK */ });

MessageBox::showError(manager, "Nie można otworzyć pliku.");

MessageBox::showWarning(manager, "Czy chcesz kontynuować?",
                        []() { /* Tak */ },
                        []() { /* Nie */ });

MessageBox::showQuestion(manager, "Zapisać zmiany?",
                         []() { /* Zapisz */ });
```

### Uwagi

- Nie dodawaj zwróconego elementu przez `addElement` — jest już dodany.
- Wewnętrznie `MessageBox` używa `DialogBox`, więc zachowuje się jak overlay.
- `showCustom` bez argumentów tekstowych przycisku pokaże przycisk „OK".

## FileDialog

Pełnoprawny dialog wyboru pliku: lewy panel z katalogami (wpis „.." wchodzi do
katalogu nadrzędnego), prawy panel z plikami (filtrowane wg wzorca), pasek
bieżącej ścieżki, pole nazwy pliku oraz przyciski akcji
(Open/Save + Cancel). Dwa tryby: `Open` i `Save`.

### Tworzenie

Fabryki **same dodają** dialog do managera i zwracają surowy wskaźnik
`FileDialog*` (nie `unique_ptr` — to wyjątek od reguły widgetów).

```cpp
enum class Mode { Open, Save };
using Callback = std::function<void(const std::string& path)>;

static FileDialog* createOpen(
    GUIManager& manager,
    std::string_view title,
    Callback callback,
    std::string_view startPath = {},
    std::string_view filter = "*.*");

static FileDialog* createSave(
    GUIManager& manager,
    std::string_view title,
    Callback callback,
    std::string_view startPath = {},
    std::string_view filter = "*.*");
```

- `title` — tytuł okna (wyświetlany w pasku tytułu).
- `callback` — wywoływana po potwierdzeniu wyboru; otrzymuje **pełną ścieżkę**
  do pliku. Po wywołaniu callbacka dialog sam się zamyka.
- `startPath` — katalog początkowy; pusty = bieżący katalog roboczy.
- `filter` — wzorzec plików, np. `"*.cpp"` (porównanie rozszerzenia,
  niewrażliwe na wielkość liter); `"*"` lub `"*.*"` = wszystkie pliki.

```cpp
FileDialog* fd = FileDialog::createOpen(
    manager, "Otwórz plik",
    [](const std::string& path) { /* path = np. "/home/user/doc.txt" */ },
    "/home/user", "*.txt");
```

### Najważniejsze metody

| Metoda | Opis |
|--------|------|
| `void setCurrentPath(std::string_view path)` | Programowa nawigacja do katalogu |
| `const std::string& getCurrentPath() const` | Bieżący katalog |
| `const std::string& getSelectedFile() const` | Nazwa zaznaczonego pliku |
| `void close()` | Zamyka dialog (oznacza do usunięcia; wymaga `cleanup()`) |
| `bool isOpen() const` | `true` dopóki dialog jest otwarty |
| `bool isOverlay() const` | Zawsze `true` — dialog jest overlayem |

### Callbacki / zdarzenia

- Tryb `Open`: `callback(path)` wywoływana tylko dla istniejącego, zwykłego
  pliku (ścieżka z katalogu + nazwy, lub absolutna jeśli wpisano absolutną).
- Tryb `Save`: `callback(path)` wywoływana bez weryfikacji istnienia pliku.
- Cancel (lub `close()`) nie wywołuje callbacka.
- Po potwierdzeniu wyboru dialog zamyka się automatycznie.

### Uwagi

- Wskaźnik `FileDialog*` jest ważny tylko dopóki dialog jest otwarty. Po
  `close()`/potwierdzeniu wyboru staje się dangling — nie zapisuj go do
  późniejszego użycia.
- Jeśli w trybie `Open` pole nazwy pliku jest puste przy kliknięciu Open,
  używane jest ostatnie zaznaczenie (`getSelectedFile()`).
- `startPath` i `setCurrentPath()` akceptują ścieżki względne i absolutne.
