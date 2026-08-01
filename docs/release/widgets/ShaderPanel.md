# ShaderPanel

Panel z własnym shaderem fragmentów GPU — zawartość panelu jest renderowana
przez niestandardowy program cieniujący SPIR-V. Służy do proceduralnych
efektów wizualnych (fale, żarzenie, desaturacja, tła shaderowe), których nie
da się osiągnąć zwykłymi widgetami.

## Przeznaczenie

`ShaderPanel` dziedziczy po `Panel`: rysuje tło i dzieci panela do tekstury
tymczasowej, po czym blituje ją na ekran przez `SDL_RenderGeometry` z
aktywnym render state'em GPU i shaderem fragmentów. Dane per klatkę (czas,
pozycja myszy) przekazuje się przez `setUniformTime`/`setUniformMouse` — są
one pakowane w kolory wierzchołków i trafiają do shadera jako
`fragColor` (location 0) i `fragTexCoord` (location 1). Shader nie sampluje
tekstury — jest w pełni proceduralny. **Wymaga renderera GPU** (patrz
`resources.md` / sekcja SDLApp).

## Wymagania

`ShaderPanel` działa tylko z GPU rendererem SDL3 (Vulkan/SPIR-V). Okno
tworzy się drugim konstruktorem `SDLApp` z backendem:

```cpp
SDLApp app("Tytuł", 800, 600, false, GPU_VULKAN);
```

Przy zwykłym (CPU) rendererze `setShader` nie utworzy shadera (loguje błąd)
— panel po prostu będzie wyglądał jak zwykły `Panel`.

## Tworzenie

```cpp
ShaderPanel(GUIManager& manager, int x, int y, int width, int height);
```

```cpp
auto panel = std::make_unique<ShaderPanel>(manager, 40, 40, 500, 300);
// ... setShader przed dodaniem ...
manager.addElement(std::move(panel));
```

## Najważniejsze metody

| Metoda | Opis |
|--------|------|
| `void setShader(const uint8_t* spirvData, size_t spirvSize)` | Ustawia shader fragmentów (SPIR-V, entrypoint `"main"`, stage fragment). Dane to binarny plik skompilowany np. przez `glslc`. W razie błędu tworzenia — log, bez wyjątku |
| `void setShaderEnabled(bool enabled)` | Włącza/wyłącza shader; wyłączony = panel rysowany bez efektu |
| `[[nodiscard]] bool isShaderEnabled() const` | Czy shader aktywny |
| `void setUniformTime(float time)` | Wartość czasu dla shadera (np. `SDL_GetTicks() / 1000.0f`) |
| `void setUniformMouse(float x, float y)` | Pozycja myszy dla shadera (dowolne współrzędne, zwykle z `SDL_EVENT_MOUSE_MOTION`) |

### Mapowanie uniformów w shaderze

| Wejście shadera | Zawartość |
|-----------------|-----------|
| `layout(location = 0) in vec4 fragColor;` | `x` = czas (`setUniformTime`), `y` = `x` myszy, `z` = `y` myszy, `w` = 1.0 |
| `layout(location = 1) in vec2 fragTexCoord;` | Współrzędne UV blitu 0..1 |
| `layout(location = 0) out vec4 outColor;` | Kolor wyjściowy |

## Przykład

Proceduralna „woda” — shader GLSL skompilowany do SPIR-V (`glslc`), panel
GPU z czasem aktualizowanym co klatkę:

```cpp
// time_water.frag:
// #version 450
// layout(location = 0) in vec4 fragColor;
// layout(location = 1) in vec2 fragTexCoord;
// layout(location = 0) out vec4 outColor;
// void main() {
//     float time = fragColor.x;
//     vec2 uv = fragTexCoord;
//     float wave = sin(uv.x * 14.0 + time * 2.5) * sin(uv.y * 11.0 - time * 1.9);
//     vec3 col = mix(vec3(0.03, 0.07, 0.2), vec3(0.1, 0.45, 0.75), 0.5 + 0.5 * wave);
//     outColor = vec4(col, 1.0);
// }
// Kompilacja: glslc time_water.frag -o time_water.spv
```

```cpp
#include "sdl_gui.hpp"
#include <fstream>

int main(int, char**) {
    try {
        SDLApp app("ShaderPanel", 600, 400, false, GPU_VULKAN);  // GPU renderer!
        GUIManager manager(app.getRenderer());
        manager.setTheme(ThemePresets::createDarkTheme());   // KONIECZNE
        manager.setWindowSize(600, 400);                     // dla anchorów

        auto panel = std::make_unique<ShaderPanel>(manager, 40, 40, 520, 280);

        std::ifstream file("time_water.spv", std::ios::binary);
        std::vector<uint8_t> spirv((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());
        if (!spirv.empty()) {
            panel->setShader(spirv.data(), spirv.size());
        }

        manager.addElement(std::move(panel));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                manager.processEvent(e);    // 1. zdarzenia
            }
            manager.update();               // 2. timery, animacje, tooltipy
            manager.cleanup();              // 3. usuwanie elementów
            SDL_SetRenderDrawColor(app.getRenderer(), 40, 42, 54, 255);
            SDL_RenderClear(app.getRenderer());
            manager.render();               // 4. rysowanie
            SDL_RenderPresent(app.getRenderer());
        }
    } catch (const std::runtime_error& ex) {
        std::fprintf(stderr, "%s\n", ex.what());
        return 1;
    }
    return 0;
}
```

Praktycznie uniformy aktualizuje się przez wskaźnik, który zostaje ważny po
przekazaniu ownership (albo przez `ElementRef` utworzony przed `std::move`):

```cpp
ShaderPanel* panel = manager.create<ShaderPanel>(40, 40, 520, 280);
// ... setShader ...
// w pętli, przed manager.render():
panel->setUniformTime(SDL_GetTicks() / 1000.0f);
```

## Uwagi

- Bez GPU renderera (`GPU_VULKAN` w konstruktorze `SDLApp`) `setShader`
  loguje błąd i nie tworzy shadera — panel zostaje zwykłym panelem.
  `GUIManager` pobiera urządzenie GPU przez `SDL_GetGPURendererDevice`.
- Shader musi być **SPIR-V**, nie GLSL — skompiluj przez `glslc` (Vulkan
  SDK) i wczytaj binarnie. Entrypoint to `"main"`.
- Shader nie może samplować tekstury zawartości — nie ma bindings do
  samplera w tym przepływie; korzystaj z `fragColor` i `fragTexCoord`.
- Uniformy ustawiasz sam, co klatkę — czas nie liczy się automatycznie.
  Typowe: `setUniformTime(SDL_GetTicks() / 1000.0f)` przed `manager.render()`.
- Renderowanie to temp-texture + blit przez geometrię — koszt rośnie z
  rozmiarem panelu; unikaj bardzo dużych `ShaderPanel` na słabym GPU.
- Wartości domyślne uniformów: czas 0.0, mysz (−1, −1) — do czasu pierwszej
  aktualizacji shader dostaje te wartości.
- `ShaderPanel` to wciąż `Panel`: działa stylizacja (motyw `"Panel"`),
  dzieci, tooltipy; dzieci wchodzą w efekt shadera (są częścią blitu).
- Po zmianie rozmiaru panelu tekstura tymczasowa jest odtwarzana
  automatycznie przy następnym renderowaniu.
