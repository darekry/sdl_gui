# Responsive Layout System — Reference Document

> **Status:** ✅ IMPLEMENTED (2026-05-22)  
> This document is kept for reference. The Anchor system is now part of the library.

## Implemented Features

### Anchor System - Opcja 2

System kotwic został zaimplementowany w bibliotece. Elementy mogą teraz automatycznie reagować na zmianę rozmiaru okna/rodzica.

---

## Implementacja

### Pliki dodane/zmodyfikowane:

| Plik | Opis |
|------|------|
| `src/anchor.hpp` | Struktura Anchor z presetami |
| `src/gui.hpp` | Metody setAnchor, applyAnchor, updateLayout |
| `src/gui.cpp` | Implementacja anchor calculations |
| `src/gui_manager.hpp` | handleResize, setResizeCallback |
| `src/gui_manager.cpp` | Resize handling logic |
| `src/sdl_app.hpp` | resizable flag, getWindowSize() |
| `examples/example_resize.cpp` | Demo anchor system |

### API:

```cpp
// W GUIElement:
void setAnchor(const Anchor& anchor);
const Anchor& getAnchor() const;
bool hasAnchor() const;
void updateLayout(int parentWidth, int parentHeight);

// W GUIManager:
void handleResize(int width, int height);
void setResizeCallback(ResizeCallback callback);
void setWindowSize(int width, int height);
```

---

## Stan obecny

Biblioteka SDL GUI używa **absolutnego pozycjonowania** - wszystkie elementy mają stałe współrzędne `(x, y, width, height)` w pikselach. Brak automatycznego przeliczania przy zmianie rozmiaru okna.

### Ograniczenia:
- Brak obsługi `SDL_WINDOWEVENT_RESIZED`
- Brak systemu kotwic (anchors)
- Brak kontenerów layoutu (flex, grid)
- Brak procentowego wymiarowania
- DialogBox ma hardcodowane wymiary ekranu 800x600

---

## Opcje implementacji

### Opcja 1: Responsywne callbacki + helpery (Najprostsza)

**Zakres:** Minimalne zmiany w bibliotece

**Implementacja:**
```cpp
// W GUIManager:
using ResizeCallback = std::function<void(int newWidth, int newHeight)>;
void setOnResize(ResizeCallback callback);
void setResizable(bool resizable);

// W GUIElement:
virtual void onParentResize(int parentWidth, int parentHeight) {}
```

**Użycie:**
```cpp
guiManager.setOnResize([&elements](int w, int h) {
    elements.updateLayout(w, h);
});
```

**Zalety:**
- Minimalne zmiany w kodzie biblioteki
- Pełna kontrol użytkownika nad layoutem
- Łatwe do zrozumienia

**Wady:**
- Każdy element musi mieć wskaźnik/tracker do aktualizacji
- Manualne zarządzanie pozycjami
- Brak deklaratywnego podejścia

**Szacowany czas:** 2-4h

---

### Opcja 2: System kotwic (Anchors) (Zalecana)

**Zakres:** Średnie zmiany w bibliotece

**Struktura danych:**
```cpp
struct Anchor {
    // Wartości 0.0-1.0 = procentowe, >1.0 = piksele
    float left = 0.0f;      // Odległość od lewej krawędzi
    float top = 0.0f;       // Odległość od górnej krawędzi
    float right = 0.0f;     // Odległość od prawej krawędzi
    float bottom = 0.0f;    // Odległość od dolnej krawędzi
    
    // Specjalne wartości:
    // -1.0 = ignotuj (użyj pozycji bezwzględnej)
    // 0.5 = środek (50%)
    
    bool isStretchedHorizontal() const { return left >= 0 && right >= 0; }
    bool isStretchedVertical() const { return top >= 0 && bottom >= 0; }
};

// W GUIElement:
void setAnchor(const Anchor& anchor);
void setAnchorLeft(float value);   // Skrót
void setAnchorRight(float value);
void setAnchorTop(float value);
void setAnchorBottom(float value);
void setAnchorCenter();             // Wyśrodkuj
void setAnchorFill(int margin = 0); // Wypełnij rodzica
```

**Przykłady użycia:**
```cpp
// Panel centrowany
panel->setAnchor({0.5f, 0.5f, -1.0f, -1.0f}); // Środek, rozmiar bezwzględny

// Przycisk w prawym dolnym rogu
button->setAnchorRight(10);
button->setAnchorBottom(10);

// Panel rozciągają się poziomo
statusBar->setAnchorLeft(0);
statusBar->setAnchorRight(0);
statusBar->setAnchorBottom(0);

// Wypełnij cały ekran z marginesami
content->setAnchorFill(10);
```

**Mechanizm aktualizacji:**
```cpp
// W GUIElement:
void updateLayout(int parentWidth, int parentHeight) {
    if (hasAnchor()) {
        applyAnchor(parentWidth, parentHeight);
    }
    // Propaguj do dzieci
    for (auto& child : m_children) {
        child->updateLayout(getWidth(), getHeight());
    }
}
```

**Zalety:**
- Deklaratywne definiowanie layoutu
- Automatyczne przeliczanie przy resize
- Elastyczne - obsługuje większość przypadków
- Może działać z istniejącym kodem (anchor opcjonalny)

**Wady:**
- Wymaga dodania obsługi resize w SDLApp
- Kotwice procentowe mogą być mylące przy zagnieżdżaniu

**Szacowany czas:** 8-12h

---

### Opcja 3: Flex Container (Średnio-zaawansowana)

**Zakres:** Nowy komponent + zmiany w systemie resize

**Struktura:**
```cpp
enum class FlexDirection { Row, Column };
enum class FlexAlign { Start, Center, End, Stretch };
enum class FlexJustify { Start, Center, End, SpaceBetween, SpaceAround };

class FlexContainer : public Panel {
    FlexDirection m_direction = FlexDirection::Row;
    FlexAlign m_align = FlexAlign::Start;
    FlexJustify m_justify = FlexJustify::Start;
    int m_gap = 4;
    int m_padding = 8;
    
    void layout() override; // Przelicz pozycje dzieci
};

// W GUIElement:
void setFlexGrow(float grow);   // 0 = nie rośnij, 1 = wypełnij
void setFlexShrink(float shrink);
void setFlexBasis(int basis);   // Bazowy rozmiar
```

**Użycie:**
```cpp
// Toolbar poziomy
auto toolbar = std::make_unique<FlexContainer>(gui);
toolbar->setDirection(FlexDirection::Row);
toolbar->setGap(8);
toolbar->setAlign(FlexAlign::Center);

toolbar->addChild(std::make_unique<Button>(gui, "New"));
toolbar->addChild(std::make_unique<Button>(gui, "Open"));
toolbar->addChild(std::make_unique<Button>(gui, "Save"));

// Sidebar pionowy
auto sidebar = std::make_unique<FlexContainer>(gui);
sidebar->setDirection(FlexDirection::Column);
sidebar->setGap(4);
```

**Zalety:**
- Intuicyjne dla programistów webowych
- Automatyczny układ elementów
- Dobra kontrola nad spacjowaniem

**Wady:**
- Ograniczone do kontenera FlexContainer
- Wymaga przeliczania przy każdej zmianie dzieci
- Bardziej złożone niż anchors

**Szacowany czas:** 12-16h

---

### Opcja 4: Grid Layout (Zaawansowana)

**Struktura:**
```cpp
class GridContainer : public Panel {
    int m_rows = 1;
    int m_cols = 1;
    std::vector<int> m_rowHeights;    // -1 = auto
    std::vector<int> m_colWidths;     // -1 = auto
    int m_gap = 4;
    
    void setGrid(int rows, int cols);
    void setChildAt(int row, int col, std::unique_ptr<GUIElement> child);
};
```

**Użycie:**
```cpp
auto grid = std::make_unique<GridContainer>(gui);
grid->setGrid(3, 2);  // 3 wiersze, 2 kolumny
grid->setChildAt(0, 0, std::make_unique<Label>(gui, "Name:"));
grid->setChildAt(0, 1, std::make_unique<TextInput>(gui, ""));
grid->setChildAt(1, 0, std::make_unique<Label>(gui, "Email:"));
grid->setChildAt(1, 1, std::make_unique<TextInput>(gui, ""));
```

**Szacowany czas:** 16-24h

---

## Rekomendowana ścieżka implementacji

### Faza 1: Podstawy (example_resize.cpp) ✅ ZROBIONE
- Obsługa `SDL_WINDOWEVENT_RESIZED` w pętli zdarzeń
- Helper functions do pozycjonowania (`centerInParent`, `anchorBottomRight`, etc.)
- Manualne wywoływanie `updateLayout()`

### Faza 2: Resize support w bibliotece
- Dodać `SDL_WINDOW_RESIZABLE` do SDLApp
- Dodać callback `onResize` w GUIManager
- Propagacja resize do dzieci

### Faza 3: System kotwic (Anchors)
- Dodać `Anchor` struct do GUIElement
- Zaimplementować `applyAnchor()` i `updateLayout()`
- Dodać skróty: `setAnchorCenter()`, `setAnchorFill()`

### Faza 4: (Opcjonalnie) Flex Container
- Zaimplementować `FlexContainer` dla automatycznego układania

---

## Przykłady zastosowań

### Dialog centrowany:
```cpp
// Z anchors:
dialog->setAnchorCenter();

// Bez anchors:
centerInParent(dialog, windowWidth, windowHeight);
```

### StatusBar na dole:
```cpp
// Z anchors:
statusBar->setAnchorLeft(0);
statusBar->setAnchorRight(0);
statusBar->setAnchorBottom(0);
statusBar->setHeight(30);

// Bez anchors:
statusBar->setPosition(0, windowHeight - 30);
statusBar->setSize(windowWidth, 30);
```

### Sidebar + Content:
```cpp
// Sidebar (fixed width, full height):
sidebar->setAnchorLeft(0);
sidebar->setAnchorTop(0);
sidebar->setAnchorBottom(0);
sidebar->setWidth(200);

// Content (fills remaining space):
content->setAnchorLeft(200);  // After sidebar
content->setAnchorRight(0);
content->setAnchorTop(0);
content->setAnchorBottom(0);
```

---

## Pytania do rozważenia

1. **Czy anchors powinny być procentowe czy pikselowe?**
   - Procentowe: `0.5` = 50% szerokości rodzica
   - Pikselowe: `10` = 10 pikseli od krawędzi
   - Hybrydowe: `Anchor::percent(0.5)` vs `Anchor::pixels(10)`

2. **Czy kontenery powinny automatycznie wywoływać layout dzieci?**
   - Tak = wygoda, ale możliwa redundancja
   - Nie = jawne `layout()` calls

3. **Jak obsłużyć min/max rozmiary?**
   - `setMinWidth()`, `setMaxWidth()` etc.
   - Potrzebne przy stretch anchors

4. **Czy dodać constraints?**
   - Aspect ratio lock
   - Minimum/maximum dimensions

---

## Pliki do modyfikacji (przy implementacji)

### Biblioteka:
- `src/gui.hpp` - dodać Anchor struct, setAnchor methods
- `src/gui.cpp` - implementacja applyAnchor, updateLayout
- `src/gui_manager.hpp/cpp` - resize callback, onResize event
- `src/sdl_app.hpp` - opcjonalnie: setResizable()

### Nowe pliki:
- `src/flex_container.hpp/cpp` - (opcjonalnie) FlexContainer
- `src/grid_container.hpp/cpp` - (opcjonalnie) GridContainer