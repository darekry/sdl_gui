# Plan: Edytor WYSIWYG dla SDL GUI

## Cel

Stworzenie wizualnego edytora layoutów GUI, który pozwala:
1. Wybierać widgety z palety i umieszczać je na kanwie przez kliknięcie
2. Edytować parametry widgetów w oknie edytora
3. Widzieć zmiany natychmiast w oknie wynikowym
4. Zapisywać layout do XML/JSON dla późniejszego wczytania przez SGMLParser/JsonParser

## Architektura

### Dwa okna (WindowManager)

```
┌─────────────────────────────────────┐    ┌─────────────────────────────────────┐
│  OKNO EDYTORA                        │    │  OKNO WYNIKOWE (Preview)             │
│                                      │    │                                      │
│  ┌─────────────────────────────────┐│    │  ┌─────────────────────────────────┐│
│  │ PALETA WIDGETÓW                 ││    │  │                                 ││
│  │ [Button] [Label] [Checkbox]     ││    │  │    CANVA (kliknij aby dodać)    ││
│  │ [TextInput] [Slider] [Panel]    ││    │  │                                 ││
│  │ [ComboBox] [TextArea] [...]     ││    │  │   [przycisk1]                   ││
│  └─────────────────────────────────┘│    │  │      [label]                    ││
│                                      │    │  │           [checkbox]            ││
│  ┌─────────────────────────────────┐│    │  │                                 ││
│  │ PARAMETRY WYBRANEGO ELEMENTU    ││    │  │                                 ││
│  │ ID: [________]                  ││    │  │                                 ││
│  │ X:  [___] Y: [___]              ││    │  │                                 ││
│  │ W: [___] H: [___]               ││    │  │                                 ││
│  │ Text: [__________]              ││    │  │                                 ││
│  │ BgColor: [R][G][B][A] (suwaki)  ││    │  │                                 ││
│  │ BorderColor: [R][G][B][A]       ││    │  │                                 ││
│  │ BorderWidth: [___]              ││    │  │                                 ││
│  │ BorderRadius: [___]             ││    │  └─────────────────────────────────┘│
│  │ FontSize: [___]                 ││    │                                      │
│  └─────────────────────────────────┘│    │                                      │
│                                      │    │                                      │
│  ┌─────────────────────────────────┐│    │                                      │
│  │ LISTA DODANYCH ELEMENTÓW        ││    │                                      │
│  │ > button1 (Button)              ││    │                                      │
│  │ > label1 (Label)                ││    │                                      │
│  │ > checkbox1 (Checkbox)          ││    │                                      │
│  │   [Delete] [Duplicate]          ││    │                                      │
│  └─────────────────────────────────┘│    │                                      │
│                                      │    │                                      │
│  [Save XML] [Save JSON] [Load]      │    │                                      │
└─────────────────────────────────────┘    └─────────────────────────────────────┘
```

### Komponenty

#### 1. EditorWindow (Window*)
- **PalettePanel**: Lista przycisków dla każdego typu widgetu
- **PropertiesPanel**: Dynamiczne pola edycji parametrów aktualnie wybranego elementu
- **ElementsList**: ListView z listą dodanych elementów + akcje (Delete, Duplicate)
- **SaveButtons**: Przyciski Save XML, Save JSON, Load

#### 2. PreviewWindow (Window*)
- **CanvasPanel**: Panel stanowiący "kanwę" edycji
  - Kliknięcie na Canvas → dodaje wybrany widget na pozycji kliknięcia
  - Widgety na Canvas są interaktywne (można przesuwać, resize?)

#### 3. EditorState (centralny stan)
```cpp
struct EditorElement {
    std::string id;
    std::string type;          // "Button", "Label", etc.
    int x, y, width, height;
    std::map<std::string, std::string> properties;
    std::vector<Style> styles;
    std::vector<std::string> childIds;  // hierarchy
};

class EditorState {
    std::vector<EditorElement> elements;
    std::string selectedWidgetType;     // aktualnie wybrany typ z palety
    size_t selectedElementIndex;        // aktualnie wybrany element na Canvas
    std::string clipboard;              // for duplicate
    
    // Komunikacja między oknami
    void addElement(int x, int y);      // z Preview → Editor
    void updateElement(size_t index);   // z Editor → Preview
    void selectElement(size_t index);   // z Preview → Editor
    void deleteElement(size_t index);
    void duplicateElement(size_t index);
};
```

### Flow interakcji

1. **Dodawanie elementu**:
   - User kliknię przycisk "Button" w Palette → `selectedWidgetType = "Button"`
   - User kliknię na Canvas w Preview → `addElement(clickX, clickY)`
   - EditorState tworzy EditorElement, PreviewWindow tworzy rzeczywisty Button
   - ElementsList aktualizuje się, PropertiesPanel pokazuje parametry nowego elementu

2. **Edycja parametrów**:
   - User edytuje pole "Text" → `updateElement()` → Preview rerenderuje
   - User zmienia kolor przez suwaki → update → Preview rerenderuje

3. **Zapis**:
   - `saveToXML()` / `saveToJSON()` → generuje plik zgodny z SGMLParser/JsonParser formatem

4. **Wczytanie**:
   - `loadFromFile()` → parsuje XML/JSON → tworzy EditorElements → renderuje Preview

## Widgety do obsługi

Zgodnie z `layout_parser.cpp`:

| Widget | Specjalne parametry |
|--------|---------------------|
| Panel | draggable, children |
| Button | text |
| Label | text, fontSize |
| Checkbox | checked |
| RadioButton | selected |
| RadioGroup | options[], optionSpacing, buttonX, labelX, startY |
| Slider | min, max, value, orientation |
| StringGrid | rowCount, colCount, showRowHeaders, showColumnHeaders, editable |
| TextInput | text, locked |
| TextArea | text, wordWrap, fontPath, fontSize |
| ComboBox | items[] |
| TabControl | tabs[], tabHeight |
| ListView | items[], rowHeight, selectedIndex |
| AnimatedImage | path, frames, rows, frameW, frameH, fps, loop, autoplay |
| Canvas | - |

## Parametry wspólne

- id, x, y, width, height
- visible, enabled
- anchor (responsive positioning)

## Style (per state: Normal, Hover, Pressed, Disabled)

- backgroundColor, textColor, borderColor
- borderWidth, borderRadius
- fontSize, fontName
- texture

## Format wyjściowy (XML)

```xml
<Layout>
  <Resources>
    <Font path="assets/fonts/font.ttf" size="16"/>
  </Resources>
  <Panel id="mainPanel" x="50" y="50" width="700" height="500" draggable="false">
    <Style state="Normal" backgroundColor="250,250,255,255" borderColor="100,100,150,255" borderWidth="2" borderRadius="10"/>
    <Button id="btn1" x="100" y="100" width="120" height="40" text="Click Me">
      <Style state="Normal" backgroundColor="200,200,220,255"/>
      <Style state="Hover" backgroundColor="220,220,240,255"/>
    </Button>
    <Label id="lbl1" x="20" y="20" text="Title" fontSize="24"/>
  </Panel>
</Layout>
```

## Format wyjściowy (JSON)

```json
{
  "resources": {
    "fonts": [{"path": "assets/fonts/font.ttf", "size": 16}]
  },
  "root": {
    "type": "Panel",
    "id": "mainPanel",
    "x": 50, "y": 50, "width": 700, "height": 500,
    "draggable": false,
    "styles": [
      {"state": "Normal", "backgroundColor": "250,250,255,255", "borderColor": "100,100,150,255", "borderWidth": 2, "borderRadius": 10}
    ],
    "children": [
      {"type": "Button", "id": "btn1", "x": 100, "y": 100, "width": 120, "height": 40, "text": "Click Me"},
      {"type": "Label", "id": "lbl1", "x": 20, "y": 20, "text": "Title", "fontSize": 24}
    ]
  }
}
```

## Pliki do utworzenia

1. `src/editor/editor_state.hpp` - centralny stan edytora
2. `src/editor/editor_state.cpp` - implementacja
3. `src/editor/editor_window.hpp` - okno edytora (palette, properties, list)
4. `src/editor/editor_window.cpp` - implementacja
5. `src/editor/preview_window.hpp` - okno wynikowe (canvas)
6. `src/editor/preview_window.cpp` - implementacja
7. `src/editor/layout_exporter.hpp` - export do XML/JSON
8. `src/editor/layout_exporter.cpp` - implementacja
9. `examples/example_wysiwyg_editor.cpp` - przykład użycia

## Implementacja - Etapy

### Etap 1: Core State & Export (2-3h)
- EditorState class z podstawową strukturą danych
- LayoutExporter dla XML i JSON
- Testy jednostkowe dla EditorState

### Etap 2: EditorWindow (3-4h)
- PalettePanel z przyciskami widgetów
- PropertiesPanel z dynamicznymi polami
- ElementsList z ListView
- Integracja z WindowManager

### Etap 3: PreviewWindow & Interaction (3-4h)
- CanvasPanel jako "drop zone"
- Click-to-add interaction
- Live update gdy parametry zmieniają się
- Selection handling (kliknięcie elementu na Canvas)

### Etap 4: Save/Load (1-2h)
- Save to XML/JSON buttons
- Load from file dialog (FileDialog composite)
- Error handling

### Etap 5: Advanced Features (opcjonalne)
- Drag-to-move na Canvas
- Resize handles
- Hierarchia (dodawanie dzieci do Panel)
- Undo/Redo
- Copy/Paste

## Decyzje implementacyjne (zatwierdzone)

| Decyzja | Wybór | Uwagi |
|---------|-------|-------|
| **Przesuwanie elementów** | ✅ Draggable | Elementy można przesuwać przez drag&drop, X/Y aktualizują się automatycznie |
| **Resize handles** | ❌ No resize handles | Rozmiar tylko przez pola edycji W/H (prostsze implementacja) |
| **Hierarchia** | ✅ Full hierarchy | Można dodać element jako child do Panel - struktura drzewa |
| **Grid/Snap** | ✅ Grid + Snap | Siatka 20px, elementy snapują do siatki przy dodawaniu i przesuwaniu |
| **Undo/Redo** | 🔄 MVP: bez Undo | Na MVP bez Undo/Redo, można dodać później |
| **Default font** | ✅ `assets/fonts/font.ttf` | Używany jako domyślny, user może zmienić w properties |
| **Drag on Canvas** | ✅ Implementacja | Kliknięcie na element na Canvas → select + allow drag-to-move |

## Dodatkowe funkcje

- **Selection highlight**: Wybrany element na Canvas ma wyróżnioną ramkę (np. blue border)
- **Snap-to-grid**: Przy dodawaniu i przesuwaniu elementy snapują do siatki 20px
- **Hierarchia**: Elementy można dodać do Panel jako children (drzewo w ElementsList)
- **Delete/Duplicate**: Przyciski na ElementsList
- **Auto ID**: `elementType` + counter (np. `button1`, `button2`, `label1`)

---

## Zaktualizowane Flow

### Dodawanie elementu (z snap)
1. User klika przycisk "Button" w Palette → `selectedWidgetType = "Button"`
2. User klika na Canvas → `addElement(clickX, clickY)` → snap do grid 20px
3. PreviewWindow tworzy Button, EditorState zapisuje EditorElement
4. ElementsList aktualizuje się, PropertiesPanel pokazuje parametry
5. Nowy element jest automatycznie selected (blue border)

### Przesuwanie elementu (drag)
1. User klika na element na Canvas → `selectElement(index)`
2. User drag element → `moveElement(index, newX, newY)` → snap do grid
3. PropertiesPanel aktualizuje X/Y pola
4. PreviewWindow rerenderuje element na nowej pozycji

### Hierarchia (children)
1. User klika Panel w ElementsList → `selectElement(panelIndex)`
2. User klika "Add as Child" button → `addChildMode = true`
3. User klika na Canvas → element jest dodany jako child do selected Panel
4. ElementsList pokazuje drzewo: Panel → Button, Label, etc.

### Zapis
1. User klika "Save XML" → FileDialog → wybiera plik
2. `LayoutExporter::saveToXML(elements, filePath)`
3. XML format zgodny z SGMLParser

---

## Zaktualizowane Pliki

1. `src/editor/editor_element.hpp` - struktura EditorElement
2. `src/editor/editor_state.hpp` - centralny stan edytora
3. `src/editor/editor_state.cpp` - implementacja
4. `src/editor/editor_window.hpp` - okno edytora
5. `src/editor/editor_window.cpp` - implementacja
6. `src/editor/preview_window.hpp` - okno wynikowe
7. `src/editor/preview_window.cpp` - implementacja (z drag handling)
8. `src/editor/layout_exporter.hpp` - export XML/JSON
9. `src/editor/layout_exporter.cpp` - implementacja
10. `examples/example_wysiwyg_editor.cpp` - example