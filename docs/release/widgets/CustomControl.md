# CustomControl

Fokusowalny `Panel` do kontrolek owner-draw. Zawsze rysuje się ścieżką
direct (bez cache'u); aplikacja instaluje hooki rysowania i/lub event hook.

```cpp
auto control = std::make_unique<CustomControl>(manager, 10, 10, 120, 40);
control->setDrawHook(DrawPhase::Replace,
    [](SDL_Renderer* r, const SDL_Rect& rect, const SDL_Rect& clip, void* data) {
        SDL_SetRenderDrawColor(r, 40, 120, 200, 255);
        SDL_RenderFillRect(r, &rect);
    });
control->setEventHook(
    [](const SDL_Event& e, int lx, int ly, void* data) { return e.type == SDL_EVENT_MOUSE_BUTTON_DOWN; });
control->notifyCommand(42);   // trafia do najbliższego FormDialog
```

Semantyka hooków:

- `DrawPhase::Before` / `After` uzupełniają domyślne rysowanie widgetu,
  `Replace` je zastępuje.
- Callback dostaje prawdziwy `SDL_Renderer*`, prostokąt lokalny `{0,0,w,h}`
  i clip przodków przetłumaczony na współrzędne lokalne (rysowanie w
  przestrzeni lokalnej).
- Element z hookiem nie korzysta z render-cache; rotacja + hook nie jest
  wspierana (hook wygrywa, rotacja jest ignorowana).
- Event hook jest wołany przed propagacją do dzieci; `1` konsumuje zdarzenie.

C API: `sdlgui_custom_control_create`, `sdlgui_element_set_draw_hook`,
`sdlgui_element_set_event_hook`, `sdlgui_element_notify_command`.
