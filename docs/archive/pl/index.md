# Dokumentacja biblioteki SDL GUI

Dokumentacja biblioteki GUI opartej na SDL2, napisanej w C++23.

## Pierwsze kroki

- [Pierwsze kroki](getting_started.md) - Konfiguracja i podstawowe użycie

## Dokumentacja widgetów

### Polski (PL)
- [AnimatedImage](animated_image.md) - Animowany widget obrazka
- [ContextMenu](context_menu.md) - Menu kontekstowe
- [Użycie SDL GUI w grach RTS](for_rts.md) - Przewodnik dla interfejsów RTS

### Angielski (EN)
- [Getting Started Guide](../getting_started.md) - Setup and basic usage
- [AnimatedImage](../en/animated_image.md) - Frame-based animations
- [ContextMenu](../en/context_menu.md) - Right-click context menus
- [Using SDL GUI for RTS Games](../en/for_rts.md) - Guide for RTS game UIs

## Dokumentacja API

- [Button](../api/Button.md) - Interaktywny przycisk
- [GUIManager](../api/GUIManager.md) - Centralny kontroler elementów GUI
- [Panel](../api/Panel.md) - Kontener dla innych elementów

## Dokumentacja techniczna

- [Mouse Cursor](../mouse_cursor.md) - Niestandardowy kursor myszy
- [Responsive Layout System](../responsive_layout_proposal.md) - System kotwic
- [Texture & Font Manager Review](../texture_font_manager_review.md) - Przegląd kodu

## Struktura projektu

| Katalog | Opis |
|---------|------|
| `src/` | Implementacja biblioteki (C++23) |
| `src/composite/` | DialogBox, MessageBox, FileDialog |
| `examples/` | 28 przykładów aplikacji |
| `tests/` | Testy jednostkowe (Catch2) |
| `docs/` | Dokumentacja (EN/PL) |