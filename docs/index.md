# SDL GUI Library Documentation

This is the documentation for the SDL2-based GUI library written in C++23.

## Getting Started

- [Getting Started Guide](getting_started.md) - Setup and basic usage

## Widget Documentation

### English (EN)
- [AnimatedImage](en/animated_image.md) - Frame-based animations from sprite sheets
- [ContextMenu](en/context_menu.md) - Right-click context menus
- [Using SDL GUI for RTS Games](en/for_rts.md) - Guide for real-time strategy game UIs

### Polish (PL)
- [Pierwsze kroki](pl/getting_started.md) - Przewodnik konfiguracji
- [AnimatedImage](pl/animated_image.md) - Animowany widget obrazka
- [ContextMenu](pl/context_menu.md) - Menu kontekstowe
- [Użycie SDL GUI w grach RTS](pl/for_rts.md) - Przewodnik dla interfejsów RTS

## API Reference

- [Button](api/Button.md) - Interactive button with keyboard support (Enter/Space)
- [Checkbox](api/Checkbox.md) - Toggle checkbox with keyboard support (Space)
- [GUIManager](api/GUIManager.md) - Central controller with Tab navigation
- [Panel](api/Panel.md) - Container widget for other elements

## Technical Documentation

- [Mouse Cursor](mouse_cursor.md) - Custom cursor system (PL)
- [Responsive Layout System](responsive_layout_proposal.md) - Anchor system for window resizing
- [Texture & Font Manager Review](texture_font_manager_review.md) - Technical code review

## Project Structure

| Directory | Description |
|-----------|-------------|
| `src/` | Library implementation (C++23) |
| `src/composite/` | DialogBox, MessageBox, FileDialog |
| `examples/` | 28 example applications |
| `tests/` | Unit tests (Catch2) |
| `docs/` | Documentation (EN/PL) |