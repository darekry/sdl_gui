# Strategia testów

## Cel i narzędzia

Głównym celem strategii testowania jest zapewnienie stabilności kluczowych komponentów i szybkie wykrywanie regresji.

- **Framework**: Catch2 (wersja amalgamated w [`lib/catch_amalgamated.hpp`](lib/catch_amalgamated.hpp))
- **Uruchamianie**: `make test` w głównym katalogu projektu

---

## Testy jednostkowe

Testy znajdują się w katalogu [`tests/`](tests/) i weryfikują logikę komponentów w izolacji.

### Infrastruktura testowa

- [`tests/test_helper.hpp`](tests/test_helper.hpp) - nagłówek pomocniczy
- [`tests/test_helper.cpp`](tests/test_helper.cpp) - implementacja środowiska testowego (inicjalizacja SDL i GUIManager bez renderowania okna)

### Pokrycie testami

| Komponent | Plik testowy | Status |
|-----------|--------------|--------|
| **Widgety** | | |
| Button | `test_button.cpp` | ✓ |
| Checkbox | `test_checkbox.cpp` | ✓ |
| ComboBox | `test_combobox.cpp` | ✓ |
| ContextMenu | `test_context_menu.cpp` | ✓ |
| Label | `test_label.cpp` | ✓ |
| Panel | `test_panel.cpp` | ✓ |
| RadioButton | `test_radio_button.cpp` | ✓ |
| RadioGroup | `test_radio_group.cpp` | ✓ |
| Slider | `test_slider.cpp` | ✓ |
| StringGrid | `test_string_grid.cpp` | ✓ |
| TabControl | `test_tab_control.cpp` | ✓ |
| TextArea | `test_text_area.cpp` | ✓ |
| TextInput | `test_text_input.cpp` | ✓ |
| AnimatedImage | `test_animated_image.cpp` | ✓ |
| Canvas | `test_canvas.cpp` | ✓ |
| **Menedżery** | | |
| FontManager | `test_font_manager.cpp` | ✓ |
| TextureManager | `test_texture_manager.cpp` | ✓ |
| TimerManager | `test_timer_manager.cpp` | ✓ |
| AnimationManager | `test_animation_manager.cpp` | ✓ |
| **Inne** | | |
| Theme | `test_theme.cpp` | ✓ |

### Brakujące testy

| Komponent | Uwagi |
|-----------|-------|
| Easing | ✓ Done - test_easing.cpp (50+ assertions) |
| Style | Struktura danych, header-only |
| Cursor | ✓ Done - test_cursor.cpp (40+ assertions) |
| SDLApp | Narzędzie, header-only |
| SGMLParser | Parser XML |
| JsonParser | Parser JSON |
| LayoutParser | Interfejs parserów |

---

## Testy integracyjne (manualne)

Katalog [`examples/`](examples/) zawiera 25 przykładów pełniących rolę testów integracyjnych.

### Kluczowe przykłady do weryfikacji

| Przykład | Cel weryfikacji |
|----------|-----------------|
| `example_button.cpp` | Podstawowa funkcjonalność i stylowanie |
| `example_window.cpp` | Zarządzanie hierarchią i dziećmi |
| `example_animated_image.cpp` | Złożone widgety, timery, animacje |
| `example_string_grid.cpp` | Sortowanie, edycja, schowek |
| `example_json_parser.cpp` | Parsowanie layoutów JSON |
| `example_performance.cpp` | Wydajność przy dużej liczbie widgetów |

### Procedura

Po znaczących zmianach w bibliotece:
1. `make examples`
2. Uruchomić kluczowe przykłady i zweryfikować wizualnie

---

## Testy wydajności

[`examples/example_performance.cpp`](examples/example_performance.cpp) służy do oceny wydajności:

- Dynamiczne dodawanie/usuwanie setek obiektów
- Obserwacja licznika FPS i czasu renderowania
- Wczesne wykrywanie wąskich gardeł

---

## Podsumowanie

Strategia opiera się na trzech filarach:

1. **Testy jednostkowe (Catch2)** - automatyczna weryfikacja logiki (25 testów, 1726+ asercji)
2. **Testy integracyjne (`examples/`)** - manualna weryfikacja wizualna (25 przykładów)
3. **Testy wydajności (`example_performance.cpp`)** - monitorowanie optymalizacji

**Wszystkie testy jednostkowe przechodzą** (weryfikacja: 2026-05-17)