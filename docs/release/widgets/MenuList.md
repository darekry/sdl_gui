# MenuList

Modalna lista w stylu MenuZ: nagłówek + siatka wierszy, wiersze disabled,
preselekcja, kod anulowania i opcjonalny timer. To nie jest `MenuBar` ani
`MenuPopup` — element jest overlayem i sam rysuje swoje wiersze.

```cpp
MenuList list(manager, 40, 40, 300, 200, "Main Menu", {
    {"New Game", 1, true},
    {"Load Game", 2, true},
    {"Preferences", 3, false},
    {"Quit", 4, true},
});
list.setColumns(2);            // pozycje w wierszu (row-major)
list.setPreselect(1);
list.setCancelId(99);
list.setOnSelect([&](int id, std::string_view) { /* ... */ });
list.setOnTick([&] { /* timer 100 ms */ });
list.setTimerMs(100);
```

- Klawiatura: strzałki, Tab/Shift+Tab, Enter, Esc, pierwsza litera.
  Wiersze disabled są pomijane, nawigacja zawija.
- Mysz: hover/press/click; klik w disabled nie zaznacza.
- `setDisabledMask(uint32_t)` — bit `i` wyłącza pozycję `i` (CivLin max 24).
- `getSelectedId()` — ostatnio aktywowany id (Enter) lub id anulowania (Esc).
- `setRowCallback` pozwala rysować wiersz własnym rendererem (np. FNT z
  CivLin); rect przekazywany w współrzędnych okna.

C API: `sdlgui_menu_list_*`.
