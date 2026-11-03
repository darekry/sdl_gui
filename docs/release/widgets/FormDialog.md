# FormDialog

Generyczny modalny host dialogu (odpowiednik okna dialogowego Win16) z
rejestrem elementów po `control id`. Dziedziczy po `ModalDialog`, więc jest
overlayem i zamyka się przez `close()/closeWithResult()`.

```cpp
FormDialog form(manager, 40, 40, 360, 160, "Options");
Button* ok = form.addButton(1, "OK", 150, 90, 80, 28);
TextInput* name = form.addEdit(2, "", 150, 20, 180, 26);
form.setOnCommand([&](int id) { if (id == 1) form.end(0); });
```

- `addWidget(id, child)` przyjmuje **tylko** elementy top-level i przepina je
  przez `GUIElement::addChild()`; rejestr nie jest właścicielem dzieci.
- `addStatic / addButton / addEdit / addCustom` tworzą i rejestrują elementy.
- `findItem / getItemRect / setItemVisible / setItemEnabled / setItemText /
  setItemRect` operują po `control id`.
- `setItemText` przywraca przypisany rect (Label nie może „uciec” z
  auto-size po zmianie tekstu).
- `end(result)` → `closeWithResult(result)`; `isClosed()`, `getResult()`.
- Tytuł rysowany jest jako pasek 30 px (pusty = brak paska). W
  przeciwieństwie do `DialogBox` formularz **nie jest przeciągalny**.
- `addButton` podłącza `on_command(id)`, ale nie wymusza zamknięcia.

C API: `sdlgui_form_*`, `sdlgui_form_create_from_items` /
`sdlgui_form_item_desc_t`, `sdlgui_dialog_close_with_result`.
