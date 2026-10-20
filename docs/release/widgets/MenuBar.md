# MenuBar / MenuPopup

Klasyczny pasek menu w stylu Win32: `File`, `Edit`, `Options ▸`, checkmarki,
radio, skróty klawiaturowe i mnemoniki (`&File` → podkreślone `F`). `MenuBar`
to widoczny pasek; `MenuPopup` to jedna rozwijana lista (używana wewnętrznie,
tworzona przez `MenuBar`).

## Przeznaczenie

Port aplikacji z `USER32.dll` na SDL GUI: zamiast `HMENU`/`AppendMenu`
budujesz drzewo `MenuItem`, a `MenuBar` obsługuje otwieranie, hover-switch,
klawiaturę i akceleratory. Popupy są top-level overlayami, więc rysują się nad
treścią aplikacji.

## Tworzenie

```cpp
MenuBar(GUIManager& manager, int x, int y, int width, int height = 0);
// height <= 0 -> wysokość z fontu (lineHeight + 8)
```

```cpp
auto bar = std::make_unique<MenuBar>(guiManager, 0, 0, windowWidth);
bar->setAnchor(Anchor::topBar(0));          // pełna szerokość, przy górnej krawędzi
MenuBar* menu = bar.get();
guiManager.addElement(std::move(bar));
```

Pierwszy `MenuBar` dodany do managera (top-level albo jako dziecko layoutu)
automatycznie staje się globalnym handlerem klawiszy. `GUIManager::setMenuBar()`
pozwala wybrać inny pasek, `getMenuBar()` go odczytać.

## Treść pod paskiem

`MenuBar` nie rezerwuje miejsca w oknie (brak globalnego content-insetu).
Wzorzec: panel treści z `Anchor::pinned` i górnym marginesem = wysokość paska:

```cpp
const int barHeight = menu->getHeight();
auto content = std::make_unique<Panel>(guiManager, 0, barHeight,
                                       windowWidth, windowHeight - barHeight);
content->setAnchor(Anchor::pinned(HAnchor::Stretch, VAnchor::Stretch,
                                  0, barHeight, 0, 0));
guiManager.addElement(std::move(content));
```

Pełny przykład: `examples/68_menu_bar.cpp`.

## Budowanie menu

`addMenu()` zwraca węzeł drzewa; dalsze pozycje dodaje się łańcuchowo.
Etykiety używają Win32 mnemonic: `&` przed znakiem, `&&` = literalny `&`.

```cpp
MenuItem& file = menu->addMenu("&File");
file.addItem("&Save Game", [&]() { save(); }).setShortcut("Ctrl+S");
file.addSeparator();
file.addItem("&Find City...", [&]() { findCity(); }).setShortcut("Ctrl+F");

MenuItem& options = file.addSubmenu("&Options");
options.addItem("&Instant Advice", [&]() { toggleAdvice(); }).setChecked(true);
options.addItem("&Sound", [&]() { toggleSound(); }).setChecked(true);

MenuItem& units = file.addSubmenu("&Units");
units.addItem("&Metric").setRadio(true);
units.addItem("&Imperial").setRadio(false);
```

| Metoda `MenuItem` | Opis |
|-------------------|------|
| `addItem(text, action = nullptr, enabled = true)` | Dodaje pozycję; zwraca ją (chain na `setShortcut` itd.) |
| `addSubmenu(text)` | Jak `addItem`, ale nazwane dla węzła z dziećmi |
| `addSeparator()` | Dodaje separator; zwraca rodzica (łańcuch zostaje na tym samym poziomie) |
| `setShortcut("Ctrl+S")` | Wyświetla skrót i rejestruje akcelerator (`Ctrl/Shift/Alt` + litera/cyfra/F1–F12/strzałki/Esc/Enter/Space/... ) |
| `setChecked(bool)` / `setRadio(bool)` | Checkmark / radio (radio czyści rodzeństwo w tym samym popupie) |
| `setEnabled(bool)` | Wyszarzenie; pozycja nie aktywuje się i jest pomijana strzałkami |
| `setCommand(id)` | Zamiast `action` — komenda dla `MenuBar::setCommandHandler()` (JSON/C API) |

Kolejność aktywacji: `action` → `command` → handler paska. Pozycja z podmenu
otwiera je zamiast wywoływać akcję. Check/radio zmieniają stan przed akcją.

## Klawiatura i mysz (zachowanie Win32)

- Kliknięcie tytułu otwiera menu; kliknięcie tego samego tytułu zamyka (toggle).
- Gdy menu jest otwarte, hover nad innym tytułem przełącza pasek.
- `Alt` wchodzi w tryb menu, `Alt+litera` otwiera menu o danym mnemoniku.
- `F10` włącza/wyłącza tryb menu; `←/→` chodzą po tytułach.
- W popupie: `↑/↓` (pomija disabled/separatory), `Home/End`, `Enter/Space`
  aktywuje, `Esc` zamyka jeden poziom (na najwyższym — całość).
- Mnemonik pozycji (`&Sound`) działa w otwartym menu: samo `S` lub `Alt+S`
  aktywuje tę pozycję (jak w Win32).
- `→` otwiera podmenu lub przechodzi do następnego menu; `←` wraca poziom wyżej
  (na najwyższym — poprzednie menu).
- Skróty globalne działają, gdy żadne menu nie jest otwarte; są sprawdzane
  **przed** fokusem widgetu (kolejność jak akceleratory Win32).
- Fokus sprzed otwarcia menu jest przywracany po zamknięciu.

## Podmenu i pozycjonowanie

Każdy węzeł z dziećmi ma własny `MenuPopup` (top-level overlay), tworzony
w momencie dodania pierwszego dziecka — drzewo można budować łańcuchowo po
`addMenu()`. Popup mierzy się z fontu i tekstów (kolumna checkmarków, skróty
wyrównane do prawej, strzałki podmenu). Przy krawędzi okna popup jest
przesuwany do środka, a podmenu odwraca się na lewą stronę.

Ograniczenia wersji 1: brak ikon/bitmap w pozycjach, brak przewijania bardzo
długich menu, podmenu otwiera się natychmiast przy hover (bez opóźnienia).
Zmiany tekstu/stanu modelu po otwarciu menu wymagają `MenuPopup::refresh()`
(`menu->getPopupFor(&item)`); przed otwarciem popup odświeża się sam.

## JSON (LayoutParser)

```json
{
  "type": "MenuBar", "x": 0, "y": 0, "width": 800,
  "menus": [
    {
      "text": "&File",
      "items": [
        { "text": "&Save Game", "shortcut": "Ctrl+S", "command": "file.save" },
        { "separator": true },
        { "text": "&Options", "items": [
            { "text": "&Sound", "checked": true, "command": "options.sound" }
        ]},
        { "text": "&Units", "items": [
            { "text": "&Metric", "radio": true, "checked": true, "command": "units.metric" },
            { "text": "&Imperial", "radio": true, "command": "units.imperial" }
        ]}
      ]
    }
  ]
}
```

Akcje w JSON to `command` — podłącz jeden handler:

```cpp
menu->setCommandHandler([](std::string_view command) {
    if (command == "file.save") save();
    else if (command == "options.sound") toggleSound();
});
```

Fixture: `tests/data/menu.json`, test: `tests/test_layout_parser.cpp`.
