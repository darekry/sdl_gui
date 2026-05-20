# Propozycja komponentów złożonych dla SDL GUI

Ten dokument zawiera propozycje innych złożonych komponentów, które mogłyby zostać dodane do katalogu `src/composite/` w przyszłości.

---

## Zaimplementowane komponenty ✓

| Komponent | Status | Opis |
|-----------|--------|------|
| **DialogBox** | ✓ Done | Okno dialogowe (Confirm, Alert, Custom) |
| **MessageBox** | ✓ Done | Statyczne helper dla Info, Error, Warning, Question |
| **FileDialog** | ✓ Done MVP | ListView dla dirs/files, ".." parent navigation |

---

## 1. FilePickerDialog (Enhanced FileDialog)

Wersja FileDialog z TreeView dla hierarchicznego widoku katalogów.

### Status: MVP zaimplementowany (ListView approach)
### Pełna wersja: wymaga TreeView widget

Okno dialogowe do wyboru plików/folderów.

### Skład
- Panel (tło)
- Label (tytuł)
- TextInput (ścieżka/pattern)
- ListView (lista plików)
- Button (OK, Cancel)

### API
```cpp
auto picker = FilePickerDialog::createOpen(manager, "Wybierz plik", "/home/user", "*.txt",
    [](std::string_view path) { /* callback */ });
manager.addElement(std::move(picker));

auto picker = FilePickerDialog::createSave(manager, "Zapisz jako", "/home/user", "dokument.txt",
    [](std::string_view path) { /* callback */ });
```

### Trudność: Średnia
- Wymaga implementacji listy plików z filesystem
- Integracja z ListView dla nawigacji

---

## 2. ColorPickerDialog

Okno dialogowe do wyboru koloru.

### Skład
- Panel (tło)
- Canvas (gradient/picker)
- Slider (Hue, Saturation, Value lub R, G, B)
- Label (wartości)
- TextInput (hex value)
- Button (OK, Cancel)

### API
```cpp
auto picker = ColorPickerDialog::create(manager, SDL_Color{255, 0, 0, 255},
    [](SDL_Color color) { /* callback */ });
manager.addElement(std::move(picker));
```

### Trudność: Średnia-Hard
- Canvas musi obsłużyć gradient i kliknięcia
- Slider zależności (HSV → RGB)

---

## 3. NumberInputDialog

Dialog z spinnerem do wprowadzania liczb.

### Skład
- Panel (tło)
- Label (tytuł/wartość)
- Button (-)
- Button (+)
- TextInput (alternatywnie)
- Slider (opcjonalnie)

### API
```cpp
auto input = NumberInputDialog::create(manager, "Ilość:", 1, 100, 50,
    [](int value) { /* callback */ });
```

### Trudność: Łatwa
- Prosty skład z dostępnych widgetów

---

## 4. ProgressDialog

Dialog pokazujący postęp operacji.

### Skład
- Panel (tło)
- Label (tytuł, status)
- Slider (progress bar)
- Button (Cancel)

### API
```cpp
auto progress = ProgressDialog::create(manager, "Kopiowanie plików...", 100);
progress->setValue(50);
progress->setStatus("Kopiowanie: plik50.txt...");
progress->setOnCancel([]() { /* abort */ });
```

### Trudność: Łatwa
- Slider jako progress bar
- API do aktualizacji wartości

---

## 5. LoginDialog

Dialog do logowania.

### Skład
- Panel (tło)
- Label (Login, Password)
- TextInput (username)
- TextInput (password - hidden)
- Checkbox (Remember me)
- Button (Login, Cancel)

### API
```cpp
auto login = LoginDialog::create(manager, "Zaloguj się",
    [](std::string_view user, std::string_view pass, bool remember) { /* callback */ });
```

### Trudność: Łatwa
- TextInput z flagą hidden dla password

---

## 6. FormDialog

Generic dialog z formularzem - definiowany przez JSON/XML.

### Skład
- Panel (tło)
- Dynamically loaded widgets (z JSON)
- Button (OK, Cancel)

### API
```cpp
auto form = FormDialog::createFromJson(manager, "config/form.json",
    [](const std::map<std::string, std::string>& values) { /* callback */ });
```

### Trudność: Średnia
- Integracja z layout parser
- Mapowanie wartości

---

## 7. ToolBar

Pasek narzędzi z przyciskami/ikonami.

### Skład
- Panel (horizontal layout)
- Button (tool buttons)
- Separator (visual)

### API
```cpp
auto toolbar = ToolBar::create(manager, {
    {"New", newIcon, []() {}},
    {"Open", openIcon, []() {}},
    {"Save", saveIcon, []() {}},
    {"separator"},
    {"Cut", cutIcon, []() {}},
});
```

### Trudność: Łatwa
- Panel z przyciskami w poziomie

---

## 8. StatusBar

Pasek statusu na dole okna.

### Skład
- Panel (bottom)
- Label (status message)
- Label (progress indicator)
- Label (position info)

### API
```cpp
auto status = StatusBar::create(manager);
status->setText("Ready");
status->setPositionInfo("Line: 10, Col: 5");
```

### Trudność: Łatwa

---

## 9. AboutDialog

Dialog z informacjami o aplikacji.

### Skład
- Panel (tło)
- AnimatedImage lub Canvas (logo/icon)
- Label (app name, version, copyright)
- Button (OK)

### API
```cpp
auto about = AboutDialog::create(manager, "MyApp", "1.0.0", "© 2026 Author", logoTexture);
```

### Trudność: Łatwa

---

## 10. TooltipWindow

Tooltip jako złożony komponent z bogatą zawartością.

### Skład
- Panel (tło)
- Label (tytuł)
- Label (opis)
- Canvas (mini preview)

### API
```cpp
TooltipWindow::show(manager, element, "Tytuł", "Szczegółowy opis...");
```

### Trudność: Łatwa-Medium
- Pozycjonowanie relative to element

---

## Priorytety implementacji

| Priorytet | Komponent | Powód |
|-----------|-----------|-------|
| **High** | ProgressDialog | Często używany, prosty |
| **High** | NumberInputDialog | Prosty, często używany |
| **Medium** | ColorPickerDialog | Useful for config apps |
| **Medium** | ToolBar | Standard UI element |
| **Medium** | StatusBar | Standard UI element |
| **Medium** | LoginDialog | Common pattern |
| **Low** | FilePickerDialog | Complex, filesystem dependency |
| **Low** | FormDialog | Requires layout parser integration |
| **Low** | AboutDialog | Simple, rarely used |
| **Low** | TooltipWindow | Enhancement of existing tooltip |

---

## Architektura komponentów złożonych

### Wzorzec projektowy

Każdy komponent złożony powinien:
1. Dziedziczyć po `Panel` (dla kontenera i draggable)
2. Używać statycznych metod `create*()` jako factory
3. Przechowywać raw pointers do dzieci dla manipulacji
4. Używać callbacków dla komunikacji z aplikacją
5. Implementować `isOverlay()` = true dla modalnych dialogów

### Struktura plików
```
src/composite/
├── dialog_box.hpp/cpp      ✓ (zaimplementowany)
├── message_box.hpp/cpp     ✓ (zaimplementowany)
├── progress_dialog.hpp/cpp (do implementacji)
├── number_input.hpp/cpp    (do implementacji)
├── color_picker.hpp/cpp    (do implementacji)
├── tool_bar.hpp/cpp        (do implementacji)
├── status_bar.hpp/cpp      (do implementacji)
└── ...
```

### Kwestie do rozważenia

1. **Modal vs Non-modal**: DialogBox jest modal (blokuje inne elementy). ProgressDialog może być non-modal.
2. **Centrowanie**: Aktualnie hardcoded 800x600. Może warto dodać `getScreenSize()` do GUIManager.
3. **Fokus**: ESC/Enter handling - już w DialogBox.
4. **Theme**: Composite components używają własnych stylów override. Może dodać `Theme::setDialogStyle()`?