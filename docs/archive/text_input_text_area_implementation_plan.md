# TextInput & TextArea Implementation Plan

**Created:** 2026-05-24
**Last Updated:** 2026-05-24
**Status:** Wave 1 Complete, Wave 2 Complete, Wave 3 Complete (TextInput + TextArea Selection)

## Overview

This document tracks the implementation of missing features in TextInput and TextArea components.

## Current State Analysis

### TextInput - Existing Features (Updated after Wave 1 + Selection Fix + Wave 2)

| Feature | Status |
|---------|--------|
| Text input (SDL_TEXTINPUT) | ✓ Implemented |
| Cursor movement (←/→) | ✓ Implemented |
| Backspace deletion | ✓ Implemented |
| Enter callback (onEnterPressed) | ✓ Implemented |
| Locked state (read-only) | ✓ Implemented |
| Focus management | ✓ Implemented |
| Blinking cursor | ✓ Implemented |
| Mouse click cursor positioning | ✓ Implemented |
| Horizontal scrolling | ✓ Implemented |
| setText/getText | ✓ Implemented |
| Callbacks (onTextChanged, onEnterPressed) | ✓ Implemented |
| UTF-8/Unicode support | ✓ Implemented |
| Disabled state | ✓ Implemented |
| Style/Theming | ✓ Implemented |
| **Copy (Ctrl+C)** | ✓ **NEW - Wave 1** |
| **Paste (Ctrl+V)** | ✓ **NEW - Wave 1** |
| **Cut (Ctrl+X)** | ✓ **NEW - Wave 1** |
| **Delete key** | ✓ **NEW - Wave 1** |
| **Selection state** | ✓ **NEW - Wave 1** |
| **Shift+Arrow selection** | ✓ **NEW - Selection Fix** |
| **Ctrl+A (Select All)** | ✓ **NEW - Selection Fix** |
| **Selection rendering (highlight)** | ✓ **NEW - Selection Fix** |
| **Mouse drag selection** | ✓ **NEW - Selection Fix** |
| **Typing replaces selection** | ✓ **NEW - Selection Fix** |
| **Home key** | ✓ **NEW - Wave 2** |
| **End key** | ✓ **NEW - Wave 2** |
| **Shift+Home/End selection** | ✓ **NEW - Wave 2** |

### TextArea - Existing Features (Updated after Wave 1 + Wave 2 + Wave 3)

| Feature | Status |
|---------|--------|
| Multi-line text | ✓ Implemented |
| Word wrap | ✓ Implemented |
| Cursor movement (←/→) | ✓ Implemented |
| Enter (new line) | ✓ Implemented |
| Backspace | ✓ Implemented |
| Mouse click cursor positioning | ✓ Implemented |
| Mouse wheel scrolling | ✓ Implemented |
| Horizontal scrolling | ✓ Implemented |
| Vertical scrolling | ✓ Implemented |
| setText/getText | ✓ Implemented |
| onTextChanged callback | ✓ Implemented |
| Blinking cursor | ✓ Implemented |
| UTF-8/Unicode support | ✓ Implemented |
| **Locked state (read-only)** | ✓ **NEW - Wave 1** |
| **Copy (Ctrl+C)** | ✓ **NEW - Wave 1** |
| **Paste (Ctrl+V)** | ✓ **NEW - Wave 1** |
| **Cut (Ctrl+X)** | ✓ **NEW - Wave 1** |
| **Delete key** | ✓ **NEW - Wave 1** |
| **Selection state** | ✓ **NEW - Wave 1** |
| **Home key (per line)** | ✓ **NEW - Wave 2** |
| **End key (per line)** | ✓ **NEW - Wave 2** |
| **Ctrl+Home (document start)** | ✓ **NEW - Wave 2** |
| **Ctrl+End (document end)** | ✓ **NEW - Wave 2** |
| **Arrow Up (line navigation)** | ✓ **NEW - Wave 2** |
| **Arrow Down (line navigation)** | ✓ **NEW - Wave 2** |
| **Page Up** | ✓ **NEW - Wave 2** |
| **Page Down** | ✓ **NEW - Wave 2** |
| **Shift+Arrow selection** | ✓ **NEW - Wave 3** |
| **Shift+Up/Down selection** | ✓ **NEW - Wave 3** |
| **Ctrl+A (Select All)** | ✓ **NEW - Wave 3** |
| **Selection rendering (highlight)** | ✓ **NEW - Wave 3** |
| **Mouse drag selection** | ✓ **NEW - Wave 3** |
| **Typing replaces selection** | ✓ **NEW - Wave 3** |
| **Multi-line selection** | ✓ **NEW - Wave 3** |

---

## Implementation Waves

### Wave 1: Clipboard + Delete + Locked (CRITICAL)

**Priority:** HIGH
**Estimated Time:** 2-3 days
**Architectural Changes:** MINIMAL
**Actual Time:** ~2 hours
**Status:** ✅ COMPLETED (2026-05-24)

| Task | TextInput | TextArea | Status | Date Completed |
|------|-----------|----------|--------|----------------|
| Copy (Ctrl+C) | ✓ Done | ✓ Done | Completed | 2026-05-24 |
| Paste (Ctrl+V) | ✓ Done | ✓ Done | Completed | 2026-05-24 |
| Cut (Ctrl+X) | ✓ Done | ✓ Done | Completed | 2026-05-24 |
| Delete key | ✓ Done | ✓ Done | Completed | 2026-05-24 |
| Locked state | ✓ Done | ✓ Done | Completed | 2026-05-24 |
| Selection members (for clipboard) | ✓ Done | ✓ Done | Completed | 2026-05-24 |

**Files Modified:**
- `src/text_input.hpp` - added selection members (m_selectionStart, m_selectionEnd, m_hasSelection), selection methods
- `src/text_input.cpp` - Ctrl+C/V/X handling, Delete key, selection helper methods
- `src/text_area.hpp` - added selection members, setLocked/isLocked methods
- `src/text_area.cpp` - Ctrl+C/V/X handling, Delete key, locked state, selection helper methods
- `tests/test_text_input.cpp` - 28 new test cases (Delete, Clipboard, Backspace with selection)
- `tests/test_text_area.cpp` - 17 new test sections (Locked, Delete, Selection, Clipboard)
- `tests/test_helper.hpp/cpp` - added createKeyEvent with modifier support

**SDL API Used:**
- `SDL_GetClipboardText()` - get clipboard content
- `SDL_SetClipboardText()` - set clipboard content
- `SDL_HasClipboardText()` - check if clipboard has text

---

### Wave 2: Navigation Keys

**Priority:** HIGH
**Estimated Time:** 1-2 days
**Architectural Changes:** MINIMAL
**Actual Time:** ~1 hour
**Status:** ✅ COMPLETED (2026-05-24)

| Task | TextInput | TextArea | Status | Date Completed |
|------|-----------|----------|--------|----------------|
| Home key (jump to start) | ✓ Done | ✓ Done | Completed | 2026-05-24 |
| End key (jump to end) | ✓ Done | ✓ Done | Completed | 2026-05-24 |
| Ctrl+Home (document start) | N/A | ✓ Done | Completed | 2026-05-24 |
| Ctrl+End (document end) | N/A | ✓ Done | Completed | 2026-05-24 |
| Arrow Up (line navigation) | N/A | ✓ Done | Completed | 2026-05-24 |
| Arrow Down (line navigation) | N/A | ✓ Done | Completed | 2026-05-24 |
| Page Up | N/A | ✓ Done | Completed | 2026-05-24 |
| Page Down | N/A | ✓ Done | Completed | 2026-05-24 |
| Shift+Home/End (selection) | ✓ Done | ✓ Done | Completed | 2026-05-24 |

**Files Modified:**
- `src/text_input.cpp` - SDLK_HOME, SDLK_END handling with Shift modifier
- `src/text_area.hpp` - added helper functions (getLineFromPosition, getColumnFromPosition, getPositionFromLineAndColumn)
- `src/text_area.cpp` - full navigation implementation (Home/End, Ctrl+Home/End, Up/Down, Page Up/Down)
- `tests/test_text_input.cpp` - 8 new test sections for Home/End
- `tests/test_text_area.cpp` - 20 new test sections (Home/End, Up/Down, Page Up/Down)

**TextArea New Helper Methods:**
```cpp
size_t getLineFromPosition(size_t pos) const;
size_t getColumnFromPosition(size_t pos) const;
size_t getPositionFromLineAndColumn(size_t line, size_t column) const;
```

---

### Wave 3: Selection System

**Priority:** HIGH
**Estimated Time:** 4-5 days
**Architectural Changes:** MEDIUM
**Actual Time:** ~2 hours
**Status:** ✅ COMPLETED (2026-05-24)

| Task | TextInput | TextArea | Status | Date Completed |
|------|-----------|----------|--------|----------------|
| Selection state (m_selectionStart, m_selectionEnd) | ✓ Done | ✓ Done | Completed | 2026-05-24 |
| Shift+Arrow selection | ✓ Done | ✓ Done | Completed | 2026-05-24 |
| Shift+Up/Down selection (multi-line) | N/A | ✓ Done | Completed | 2026-05-24 |
| Ctrl+A (Select All) | ✓ Done | ✓ Done | Completed | 2026-05-24 |
| Selection rendering (highlight) | ✓ Done | ✓ Done | Completed | 2026-05-24 |
| Mouse drag selection | ✓ Done | ✓ Done | Completed | 2026-05-24 |
| Typing replaces selection | ✓ Done | ✓ Done | Completed | 2026-05-24 |
| Double-click word selection | TODO | TODO | Pending | - |

**New Member Variables:**
```cpp
// Both TextInput and TextArea:
size_t m_selectionStart = 0;
size_t m_selectionEnd = 0;
bool m_hasSelection = false;
bool m_isDragging = false;
size_t m_dragStartPos = 0;
```

**Files Modified:**
- `src/text_input.hpp` - added m_isDragging, m_dragStartPos
- `src/text_input.cpp` - Shift+Arrow, Ctrl+A, mouse drag, selection render, clip rect
- `src/text_area.hpp` - added m_isDragging, m_dragStartPos
- `src/text_area.cpp` - Shift+Arrow/Up/Down, Ctrl+A, mouse drag, multi-line selection render
- `tests/test_text_input.cpp` - selection tests
- `tests/test_text_area.cpp` - 29 new test sections (Shift+Arrow, Ctrl+A, Multi-line, Typing replaces)

---

### Wave 4: UX Improvements

**Priority:** MEDIUM
**Estimated Time:** 1-2 days
**Architectural Changes:** MEDIUM

| Task | TextInput | TextArea | Status | Date Completed |
|------|-----------|----------|--------|----------------|
| Placeholder text | TODO | TODO | Pending | - |
| Max length limit | TODO | TODO | Pending | - |
| Password mode (character masking) | TODO | N/A | Pending | - |
| Triple-click line selection | TODO | TODO | Pending | - |
| Tab handling | N/A | TODO | Pending | - |

**New Member Variables:**
```cpp
std::string m_placeholder;
size_t m_maxLength = 0;  // 0 = unlimited
bool m_passwordMode = false;
char m_passwordChar = '*';
```

**New Methods:**
```cpp
void setPlaceholder(std::string_view text);
void setMaxLength(size_t length);
void setPasswordMode(bool enabled, char maskChar = '*');
```

**Files to Modify:**
- `src/text_input.hpp` - new members and methods
- `src/text_input.cpp` - placeholder render, max length check, password masking
- `src/text_area.hpp` - placeholder, max length
- `src/text_area.cpp` - placeholder render, max length check, Tab handling
- `tests/test_text_input.cpp` - placeholder, max length, password tests
- `tests/test_text_area.cpp` - placeholder, max length, tab tests

---

### Wave 5: Undo/Redo System (Optional)

**Priority:** LOW
**Estimated Time:** 2-3 days
**Architectural Changes:** MEDIUM

| Task | TextInput | TextArea | Status | Date Completed |
|------|-----------|----------|--------|----------------|
| Undo (Ctrl+Z) | TODO | TODO | Pending | - |
| Redo (Ctrl+Y / Ctrl+Shift+Z) | TODO | TODO | Pending | - |
| History stack management | TODO | TODO | Pending | - |

**Architectural Changes:**
- New class `TextHistory` or internal history stack
- Store snapshots of text + cursor position
- Limit history depth (e.g., 50 entries)

---

## Summary Timeline

| Wave | Features | Time | Status |
|------|----------|------|--------|
| Wave 1 | Clipboard, Delete, Locked | 2-3 days | ✅ COMPLETED |
| Wave 2 | Navigation keys | 1-2 days | ✅ COMPLETED |
| Wave 3 | Selection system | 4-5 days | ✅ COMPLETED |
| Wave 4 | UX improvements | 1-2 days | Pending |
| Wave 5 | Undo/Redo | 2-3 days | Pending (Optional) |
| **Total** | | **10-15 days** | **Wave 1+2+3 Done** |

---

## Test Results

**Wave 3 Final Test Results (2026-05-24):**

| Test File | Assertions | Test Cases | Status |
|-----------|------------|------------|--------|
| test_text_input | 144 | 18 | ✅ PASSED |
| test_text_area | 178 | 24 | ✅ PASSED |

---

## Implementation Notes

### Clipboard Implementation Pattern

```cpp
// In handleEvent() for SDL_KEYDOWN:
case SDLK_c:
    if (e.key.keysym.mod & KMOD_CTRL) {
        if (hasSelection()) {
            std::string selected = m_text.substr(m_selectionStart, m_selectionEnd - m_selectionStart);
            SDL_SetClipboardText(selected.c_str());
        }
        return true;
    }
    break;

case SDLK_v:
    if (e.key.keysym.mod & KMOD_CTRL) {
        if (SDL_HasClipboardText()) {
            char* clipboard = SDL_GetClipboardText();
            if (clipboard) {
                // Delete selection if exists, then insert
                if (hasSelection()) {
                    m_text.erase(m_selectionStart, m_selectionEnd - m_selectionStart);
                    m_cursor_pos = m_selectionStart;
                    clearSelection();
                }
                m_text.insert(m_cursor_pos, clipboard);
                m_cursor_pos += strlen(clipboard);
                refreshTextTexture();
                SDL_free(clipboard);
                if (m_onTextChanged) m_onTextChanged(this);
            }
        }
        return true;
    }
    break;

case SDLK_x:
    if (e.key.keysym.mod & KMOD_CTRL) {
        if (hasSelection()) {
            std::string selected = m_text.substr(m_selectionStart, m_selectionEnd - m_selectionStart);
            SDL_SetClipboardText(selected.c_str());
            m_text.erase(m_selectionStart, m_selectionEnd - m_selectionStart);
            m_cursor_pos = m_selectionStart;
            clearSelection();
            refreshTextTexture();
            if (m_onTextChanged) m_onTextChanged(this);
        }
        return true;
    }
    break;

case SDLK_DELETE:
    if (hasSelection()) {
        m_text.erase(m_selectionStart, m_selectionEnd - m_selectionStart);
        m_cursor_pos = m_selectionStart;
        clearSelection();
    } else if (m_cursor_pos < m_text.length()) {
        m_text.erase(m_cursor_pos, 1);
    }
    refreshTextTexture();
    if (m_onTextChanged) m_onTextChanged(this);
    return true;
```

### Selection Rendering Pattern

```cpp
// In renderOverlay() after cursor blink check:
if (hasSelection()) {
    // Calculate selection rectangle(s)
    int startX = getTextWidthBeforePosition(m_selectionStart);
    int endX = getTextWidthBeforePosition(m_selectionEnd);
    
    SDL_Rect selectionRect = {
        abs_pos.x + 5 + startX + m_text_offset_x,
        abs_pos.y + (getHeight() - lineHeight) / 2,
        endX - startX,
        lineHeight
    };
    
    SDL_SetRenderDrawColor(renderer, 100, 100, 255, 80);  // Semi-transparent blue
    SDL_RenderFillRect(renderer, &selectionRect);
}
```

---

## Test Checklist

### Wave 1 Tests
- [x] Copy with empty selection does nothing
- [x] Copy with selection copies to clipboard
- [x] Paste inserts clipboard content at cursor
- [x] Paste replaces selection
- [x] Cut copies and deletes selection
- [x] Delete removes character after cursor
- [x] Delete removes selection
- [x] Locked TextArea ignores all input
- [x] Clipboard handles UTF-8
- [x] Backspace removes selection
- [x] Selection helper methods (hasSelection, getSelection, clearSelection, setSelection)
- [x] setSelection clamps to text length
- [x] Locked state prevents typing/backspace

### Wave 2 Tests
- [x] Home moves cursor to start (TextInput)
- [x] End moves cursor to end (TextInput)
- [x] Shift+Home/End creates selection (TextInput)
- [x] Home moves cursor to line start (TextArea)
- [x] End moves cursor to line end (TextArea)
- [x] Ctrl+Home moves to document start (TextArea)
- [x] Ctrl+End moves to document end (TextArea)
- [x] Arrow Up/Down navigation in TextArea
- [x] Page Up/Down scrolling in TextArea
- [x] Shift+Page Up/Down creates selection (TextArea)

### Wave 3 Tests
- [x] Shift+Arrow creates selection (TextInput)
- [x] Ctrl+A selects all (TextInput)
- [x] Selection renders correctly (TextInput)
- [x] Mouse drag creates selection (TextInput)
- [x] Backspace/Delete removes selection
- [x] Typing replaces selection
- [x] Shift+Arrow creates selection (TextArea)
- [x] Ctrl+A selects all (TextArea)
- [x] Selection renders correctly (TextArea)
- [x] Mouse drag creates selection (TextArea)
- [x] Shift+Up/Down selection (TextArea)
- [x] Multi-line selection
- [x] Typing replaces selection (TextArea)
- [x] Enter replaces selection with newline
- [ ] Double-click selects word (Pending)

### Wave 4 Tests
- [ ] Placeholder shows when empty
- [ ] Placeholder hides when focused/text entered
- [ ] Max length prevents further input
- [ ] Password mode masks characters
- [ ] Tab inserts tab character in TextArea

---

## Progress Log

| Date | Wave | Task | Notes |
|------|------|------|-------|
| 2026-05-24 | Planning | Document created | Initial plan written |
| 2026-05-24 | Wave 1 | TextInput clipboard+delete | Ctrl+C/V/X, Delete, selection members implemented |
| 2026-05-24 | Wave 1 | TextArea clipboard+delete+locked | Ctrl+C/V/X, Delete, locked state, selection members implemented |
| 2026-05-24 | Wave 1 | Tests added | 28 TextInput tests, 17 TextArea test sections added |
| 2026-05-24 | Wave 1 | All tests verified | 1,933 assertions, 204 test cases - all passed |
| 2026-05-24 | Wave 1 | Plan updated | Document updated with completion status |
| 2026-05-24 | Selection Fix | TextInput selection rendering | Added highlight rect in renderOverlay() |
| 2026-05-24 | Selection Fix | TextInput Shift+Arrow | Added Shift modifier handling for Left/Right arrows |
| 2026-05-24 | Selection Fix | TextInput Ctrl+A | Added Select All functionality |
| 2026-05-24 | Selection Fix | TextInput mouse drag | Added SDL_MOUSEMOTION/SDL_MOUSEBUTTONUP handling |
| 2026-05-24 | Selection Fix | Tests added | 9 new test sections for selection features |
| 2026-05-24 | Selection Fix | All tests verified | 135 assertions in 17 test cases - all passed |
| 2026-05-24 | Wave 2 | TextInput Home/End | Added Home, End, Shift+Home, Shift+End |
| 2026-05-24 | Wave 2 | TextArea navigation | Home/End, Ctrl+Home/End, Up/Down, Page Up/Down, helper functions |
| 2026-05-24 | Wave 2 | Tests added | 8 TextInput tests, 20 TextArea test sections for navigation |
| 2026-05-24 | Wave 2 | All tests verified | 156 TextInput, 160 TextArea assertions - all passed |
| 2026-05-24 | Wave 2 | Plan updated | Document updated with Wave 2 completion |
| 2026-05-24 | Wave 3 | TextArea selection rendering | Multi-line highlight rect in renderOverlay() |
| 2026-05-24 | Wave 3 | TextArea Shift+Arrow | Shift+Left/Right selection creation |
| 2026-05-24 | Wave 3 | TextArea Ctrl+A | Select All functionality |
| 2026-05-24 | Wave 3 | TextArea mouse drag | SDL_MOUSEMOTION/SDL_MOUSEBUTTONUP handling |
| 2026-05-24 | Wave 3 | TextArea typing replaces selection | TEXTINPUT and Enter replace selection |
| 2026-05-24 | Wave 3 | Tests added | 29 new TextArea test sections for selection |
| 2026-05-24 | Wave 3 | All tests verified | 144 TextInput, 178 TextArea assertions - all passed |
| 2026-05-24 | Wave 3 | Plan updated | Document updated with Wave 3 completion - Selection fully implemented |