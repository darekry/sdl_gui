# Krótkie podsumowanie projektu SDL GUI

SDL GUI to lekka biblioteka GUI oparta na SDL2, zapewniająca podstawowe widgety i menedżery zasobów.

## Główne cele
- Ułatwić tworzenie narzędzi i prototypów desktopowych
- Dostarczyć prosty, ergonomiczny API dla programistów C++

## Kluczowe elementy

### Core
- **GUIManager** - zarządzanie kontekstem i renderowaniem
- **GUIElement** - hierarchia elementów z cache'em tekstur

### Menedżery zasobów
- **TextureManager**, **FontManager**, **TimerManager**, **AnimationManager**

### Widgety
Panel, Button, Label, Checkbox, RadioButton, RadioGroup, Slider, StringGrid, ListView, TextInput, TextArea, ComboBox, TabControl, AnimatedImage, Canvas, ContextMenu

### Komponenty złożone (Composite)
- **DialogBox**, **MessageBox**, **FileDialog** - gotowe dialogi w katalogu `src/composite/`

### Parsery layoutów
- **JsonParser**, **SGMLParser** - definicja GUI z plików JSON/XML

### Style i motywy
- **Style**, **Theme** - spójny system wyglądu

## Struktura projektu
- [`src/`](src/) - implementacja (C++23, moduły)
- [`docs/`](docs/) - dokumentacja (EN/PL)
- [`examples/`](examples/) - przykłady użycia
- [`tests/`](tests/) - testy jednostkowe (Catch2)