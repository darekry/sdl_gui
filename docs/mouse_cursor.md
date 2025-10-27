# MouseCursor Component

## Przegląd

Komponent `MouseCursor` umożliwia tworzenie niestandardowych kursorów myszy z obsługą wielu stanów i animacji. Jest to komponent na poziomie systemu (nie dziedziczy po `GUIElement`), który integruje się z `GUIManager` i jest renderowany na wierzchu wszystkich innych elementów GUI.

## Kluczowe cechy

- **Wiele stanów kursora**: Normal, Hover, Pressed, Disabled, Busy, Text + 3 custom
- **Animowane kursory**: Obsługa sprite sheet'ów z animacją klatka po klatce
- **Integracja z istniejącymi systemami**: Wykorzystuje `TextureManager` do zarządzania teksturami
- **Hotspot**: Możliwość ustawienia punktu odniesienia kursora
- **Skalowanie**: Dynamiczne skalowanie rozmiaru kursora
- **Callback'i**: Powiadomienia o zmianie stanu kursora

## Podstawowe użycie

### 1. Włączenie niestandardowego kursora

```cpp
SDLApp app("Moja Aplikacja", 800, 600);
GUIManager gui(app.getRenderer());

// Włącz niestandardowy kursor (ukrywa systemowy kursor SDL)
gui.setCustomCursorEnabled(true);

// Pobierz wskaźnik do obiektu MouseCursor
MouseCursor* cursor = gui.getMouseCursor();
```

### 2. Ustawianie statycznych tekstur kursora

```cpp
// Ustaw teksturę dla stanu Normal z hotspot'em (8, 8)
cursor->setCursorTexture(CursorState::Normal, "assets/cursor_normal.png", 8, 8);

// Ustaw teksturę dla stanu Hover z hotspot'em (16, 16)
cursor->setCursorTexture(CursorState::Hover, "assets/cursor_hover.png", 16, 16);

// Ustaw teksturę dla stanu Disabled
cursor->setCursorTexture(CursorState::Disabled, "assets/cursor_disabled.png", 8, 8);
```

### 3. Ustawianie animowanych kursorów

```cpp
// Animowany kursor "Busy" z 8 klatkami w 2 rzędach (sprite sheet 4x2)
// Animacja z prędkością 12 FPS, hotspot (16, 16)
cursor->setAnimatedCursor(CursorState::Busy, "assets/cursor_busy.png", 8, 2, 12.0f, 16, 16);
```

### 4. Zmiana stanu kursora

```cpp
// Ręczna zmiana stanu
cursor->setState(CursorState::Hover);
cursor->setState(CursorState::Busy);
cursor->setState(CursorState::Normal);
```

### 5. Automatyczna zmiana stanu na podstawie elementu GUI

```cpp
// W głównej pętli aplikacji, przy obsłudze SDL_MOUSEMOTION
if (e.type == SDL_MOUSEMOTION) {
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    
    GUIElement* element = gui.findElementAt(mouseX, mouseY);
    if (element) {
        const char* type = element->getComponentType();
        if (std::string(type) == "Button") {
            cursor->setState(CursorState::Hover);
        } else if (std::string(type) == "TextInput") {
            cursor->setState(CursorState::Text);
        }
    } else {
        cursor->setState(CursorState::Normal);
    }
}
```

## API

### Konstruktor i destruktor

```cpp
explicit MouseCursor(GUIManager& manager);
~MouseCursor();
```

### Ustawianie tekstur

```cpp
// Ustaw statyczną teksturę dla stanu
void setCursorTexture(CursorState state, const std::string& path, 
                     int hotspotX = 0, int hotspotY = 0);

// Ustaw animowaną teksturę (sprite sheet)
void setAnimatedCursor(CursorState state, const std::string& path, 
                      int totalFrames, int rows = 1, 
                      float fps = 12.0f, 
                      int hotspotX = 0, int hotspotY = 0);
```

**Parametry:**
- `state`: Stan kursora (Normal, Hover, Pressed, itp.)
- `path`: Ścieżka do pliku tekstury
- `hotspotX`, `hotspotY`: Punkt odniesienia kursora (np. czubek strzałki)
- `totalFrames`: Liczba klatek animacji w sprite sheet
- `rows`: Liczba rzędów w sprite sheet (kolumny obliczane automatycznie)
- `fps`: Prędkość animacji w klatkach na sekundę

### Stan kursora

```cpp
void setState(CursorState state);
CursorState getState() const;
```

### Widoczność

```cpp
void setVisible(bool visible);
bool isVisible() const;
```

### Pozycja i wygląd

```cpp
// Dodatkowe przesunięcie względem pozycji myszy
void setOffset(int offsetX, int offsetY);
void getOffset(int& offsetX, int& offsetY) const;

// Skalowanie kursora (1.0 = 100%)
void setScale(float scale);
float getScale() const;
```

### Aktualizacja i renderowanie

```cpp
// Aktualizuj animacje (wywoływane automatycznie przez GUIManager)
void update();

// Renderuj kursor (wywoływane automatycznie przez GUIManager)
void render(SDL_Renderer* renderer);
```

### Callback'i

```cpp
// Callback wywoływany przy zmianie stanu kursora
void setOnStateChanged(std::function<void(CursorState)> callback);
```

## Stany kursora

```cpp
enum class CursorState {
    Normal,    // Standardowy kursor (strzałka)
    Hover,     // Kursor nad interaktywnym elementem (np. dłoń)
    Pressed,   // Kursor przy kliknięciu
    Disabled,  // Kursor nad wyłączonym elementem
    Busy,      // Kursor oczekiwania/ładowania
    Text,      // Kursor edycji tekstu (I-beam)
    Custom1,   // Niestandardowy stan 1
    Custom2,   // Niestandardowy stan 2
    Custom3    // Niestandardowy stan 3
};
```

## Format sprite sheet dla animacji

Sprite sheet powinien zawierać wszystkie klatki animacji ułożone w siatkę:
- Klatki ułożone od lewej do prawej, od góry do dołu
- Wszystkie klatki muszą mieć ten sam rozmiar
- Liczba kolumn obliczana automatycznie: `cols = ceil(totalFrames / rows)`

Przykład dla 8 klatek w 2 rzędach (4x2):
```
┌──────┬──────┬──────┬──────┐
│ kl.1 │ kl.2 │ kl.3 │ kl.4 │
├──────┼──────┼──────┼──────┤
│ kl.5 │ kl.6 │ kl.7 │ kl.8 │
└──────┴──────┴──────┴──────┘
```

## Przykład kompletnej aplikacji

```cpp
#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "mouse_cursor.hpp"
#include "button.hpp"

int main() {
    SDLApp app("Przykład kursora", 800, 600);
    GUIManager gui(app.getRenderer());
    
    // Włącz niestandardowy kursor
    gui.setCustomCursorEnabled(true);
    MouseCursor* cursor = gui.getMouseCursor();
    
    // Skonfiguruj kursory
    cursor->setCursorTexture(CursorState::Normal, "assets/cursor_arrow.png", 8, 8);
    cursor->setCursorTexture(CursorState::Hover, "assets/cursor_hand.png", 16, 16);
    cursor->setAnimatedCursor(CursorState::Busy, "assets/cursor_loading.png", 8, 2, 12.0f);
    cursor->setScale(0.75f);
    
    // Callback na zmianę stanu
    cursor->setOnStateChanged([](CursorState state) {
        std::cout << "Zmieniono stan kursora na: " << static_cast<int>(state) << "\n";
    });
    
    // Dodaj przyciski do testowania
    auto btn1 = std::make_unique<Button>(gui, 100, 100, 200, 50, "Hover me!");
    gui.addElement(std::move(btn1));
    
    auto btn2 = std::make_unique<Button>(gui, 100, 200, 200, 50, "Set Busy");
    btn2->setOnClickCallback([cursor](GUIElement*) {
        cursor->setState(CursorState::Busy);
    });
    gui.addElement(std::move(btn2));
    
    // Główna pętla
    bool quit = false;
    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = true;
            gui.processEvent(e);
            
            // Automatyczna zmiana stanu na podstawie elementu
            if (e.type == SDL_MOUSEMOTION) {
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                GUIElement* elem = gui.findElementAt(mx, my);
                if (elem && std::string(elem->getComponentType()) == "Button") {
                    cursor->setState(CursorState::Hover);
                } else if (cursor->getState() != CursorState::Busy) {
                    cursor->setState(CursorState::Normal);
                }
            }
        }
        
        gui.cleanup();
        
        SDL_SetRenderDrawColor(app.getRenderer(), 240, 240, 240, 255);
        SDL_RenderClear(app.getRenderer());
        gui.render();
        SDL_RenderPresent(app.getRenderer());
    }
    
    return 0;
}
```

## Wskazówki

1. **Hotspot**: Ustawiaj hotspot na właściwym punkcie kursora (np. czubek strzałki, środek dla kursora busy)
2. **Skalowanie**: Użyj `setScale()` aby dostosować rozmiar kursora do Twojej aplikacji
3. **Animacje**: Dla płynnych animacji używaj FPS 12-24
4. **Wydajność**: Animacje są aktualizowane co klatkę, więc nie wpływają negatywnie na wydajność
5. **Integracja**: Kursor jest automatycznie renderowany na końcu `GUIManager::render()`, więc jest zawsze na wierzchu

## Zobacz też

- `examples/example_mouse_cursor.cpp` - Pełny przykład użycia
- `GUIManager::findElementAt()` - Znajdowanie elementu pod kursorem
- `GUIElement::getComponentType()` - Identyfikacja typu elementu
