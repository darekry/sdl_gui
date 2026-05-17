# Code Review: System zarządzania teksturami i fontami

## 1. Architektura systemu

### 1.1 TextureManager (`src/texture_manager.hpp/cpp`)

TextureManager implementuje cache tekstur z następującymi elementami:

- **Mapa cache**: `std::map<std::string, SharedTexture>` - przechowuje załadowane tekstury
- **SharedTexture**: `std::shared_ptr<SDL_Texture>` z `SDLTextureDeleter` - współdzielona tekstura
- **Metody główne**:
  - `loadTexture(path)` - ładowanie obrazów z plików (PNG, etc.)
  - `createTextureFromText(text, font, color)` - tworzenie tekstur tekstowych
  - `addTexture(key, texture)` - dodawanie własnych tekstur
  - `getTexture(key)` - pobieranie tekstury z cache

### 1.2 FontManager (`src/font_manager.hpp/cpp`)

FontManager implementuje cache fontów:

- **Mapa cache**: `std::map<FontKey, SharedFont, FontCacheKeyCompare>`
- **FontKey**: `std::pair<std::string, int>` - (ścieżka, rozmiar)
- **Transparent comparator**: pozwala na wyszukiwanie bez tworzenia `std::string`
- **SharedFont**: `std::shared_ptr<TTF_Font>` z `TTFFontDeleter`

### 1.3 GUIElement Cache (`src/gui.hpp/cpp`)

Każdy element GUI posiada własny cache renderowania:

```cpp
std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> m_cachedTexture{nullptr, SDL_DestroyTexture};
bool m_isDirty = true;
```

Mechanizm renderowania:
1. Jeśli `m_isDirty`, wywołaj `renderToCache()` - rysuje na teksturę bufora
2. Kopiuj bufor na renderer przez `SDL_RenderCopy`
3. Dzieci renderowane rekurencyjnie

---

## 2. Analiza użycia tekstur w widgetach

### 2.1 Label (`src/label.cpp`)

```cpp
void Label::draw(SDL_Renderer* renderer) {
    SharedTexture textTexture = textureManager.createTextureFromText(m_text, font, color);
    SDL_RenderCopy(renderer, textTexture.get(), nullptr, &dstRect);
}
```

**Status**: ✅ POPRAWNE
- Tekstura tworzona przez `createTextureFromText()` - automatycznie cache'owana
- Label nie przechowuje lokalnie SharedTexture - używa z TextureManager
- Jeśli ten sam tekst/font/kolor jest użyty w wielu Labelach, współdzielą teksturę

### 2.2 TextInput (`src/text_input.cpp`)

```cpp
void TextInput::draw(SDL_Renderer* renderer) {
    auto text_texture = m_manager.getTextureManager().createTextureFromText(m_text, font, style.textColor.value());
    SDL_RenderCopy(renderer, text_texture.get(), nullptr, &renderQuad);
}

// Problem w handleEvent():
if (hasKeyboardFocus()) {
    markDirty(); // Keep dirty for cursor blinking
}
```

**Status**: ⚠️ PROBLEM - Blinking cursor wymusza recreate cache każdej klatki
- `markDirty()` w każdej klatce gdy TextInput ma fokus - dla migania kursora
- To wymusza `renderToCache()` każdej klatki, tworząc nową teksturę bufora
- Tekstura tekstu jest cache'owana przez TextureManager, ale cache elementu jest recreate'owany

**Rekomendacja**: Cursor powinien być rysowany bezpośrednie (bez cache) lub z oddzielnym timerem.

### 2.3 TextArea (`src/text_area.hpp/cpp`)

```cpp
std::vector<std::shared_ptr<SDL_Texture>> m_line_textures;

void TextArea::refreshTextures() {
    m_line_textures.clear();  // ⚠️ Czyści lokalny wektor, ale NIE usuwa z TextureManager cache!
    for (const auto& line : m_lines) {
        auto lineTexture = m_manager.getTextureManager().createTextureFromText(line, font, color);
        m_line_textures.push_back(lineTexture);
    }
}
```

**Status**: ❌ PROBLEM - Tekstury linii pozostają w TextureManager cache
- `m_line_textures.clear()` usuwa tylko lokalne referencje SharedTexture
- Tekstury pozostają w `m_textureCache` TextureManager na zawsze
- Jeśli TextArea zmienia tekst często, cache TextureManager rozrasta się bez limitu

**Rekomendacja**: 
1. Implementacja `TextureManager::clearUnusedTextures()` 
2. Lub przechowywanie tekstur linii bez użycia TextureManager cache

### 2.4 ComboBox (`src/combobox.cpp`)

```cpp
void ComboBox::draw(SDL_Renderer* renderer) {
    SharedTexture textTexture = textureManager.createTextureFromText(m_options[m_selected_index], font, *style.textColor);
    SDL_RenderCopy(renderer, textTexture.get(), nullptr, &dstRect);
}
```

**Status**: ✅ POPRAWNE
- Tekstura tworzona przez `createTextureFromText()` - cache'owana
- Współdzielenie tekstur dla identycznych opcji

### 2.5 StringGrid (`src/string_grid.cpp`)

```cpp
class StringGrid : public Panel {
    [[nodiscard]] bool wantsDirectRender() const override { return true; }
    void drawDirect(SDL_Renderer* renderer) override;
};

void StringGrid::drawCell(...) {
    auto texture = m_manager.getTextureManager().createTextureFromText(m_data[row][col], font, textColor);
    SDL_RenderCopy(renderer, texture.get(), nullptr, &destRect);
}
```

**Status**: ⚠️ CZĘŚCIOWY PROBLEM
- `wantsDirectRender() = true` - rysuje bezpośrednie, bez buforowania elementu
- Każda klatka: dla każdej widocznej komórki - `createTextureFromText()`
- TextureManager cache zapobiega duplicate, ale:
  - Duże siatki mogą tworzyć tysiące wpisów w cache
  - Cache rozrasta się bez limitu gdy dane zmieniają się często
  - Nagłówki wierszy (`std::to_string(row + 1)`) są tworzone każdej klatki

**Rekomendacja**: Implementacja limitu cache lub cleanup mechanizm.

### 2.6 Button / Checkbox / RadioButton / Panel

```cpp
void Button::draw(SDL_Renderer* renderer) {
    drawBackgroundAndBorder(renderer);
    if (resolvedStyle.texture.has_value()) {
        SDL_RenderCopy(renderer, resolvedStyle.texture.value().get(), nullptr, &destRect);
    }
}
```

**Status**: ✅ POPRAWNE dla tekstur z Style
- Tekstury z Style są SharedTexture - współdzielone
- Checkbox/RadioButton używają `style.texture` dla "ptaszka"/wskaźnika
- Bez problemów z recreate

---

## 3. Kluczowe problemy i błędy

### 3.1 ❌ Brak cleanup dla TextureManager cache

**Problem**: Tekstury tworzone przez `createTextureFromText()` pozostają w cache na zawsze.

```cpp
// texture_manager.cpp:57-94
SharedTexture TextureManager::createTextureFromText(...) {
    std::string cacheKey = ...;
    auto it = m_textureCache.find(cacheKey);
    if (it != m_textureCache.end()) {
        return it->second;  // Zwraca z cache - OK
    }
    // Tworzy nową teksturę i dodaje do cache
    m_textureCache.emplace(std::move(cacheKey), sharedTexture);
    return sharedTexture;
}
```

**Implikacje**:
- TextArea z dynamicznym tekstem rozrasta cache bez limitu
- StringGrid z edycją inline tworzy nowe tekstury dla każdej edycji
- Aplikacja długotrwała z dynamicznymi tekstami → memory leak

**Rekomendacja**:
```cpp
// Dodatkowe metody w TextureManager:
void clearCache();                    // Czyści cały cache
void removeTexture(std::string_view key);  // Usuwa konkretną teksturę
void pruneUnused();                   // Usuwa tekstury z use_count == 1
size_t getCacheSize() const;          // Do debugowania
```

### 3.2 ⚠️ Font pointer jako część klucza cache

```cpp
// texture_manager.cpp:65
std::string cacheKey = std::string(text) + "|" + 
    std::to_string(reinterpret_cast<uintptr_t>(font.get())) + "|" + ...;
```

**Problem**: Adres wskaźnika fontu jako część klucza.
- Jeśli font zostanie usunięty z FontManager cache i załadowany ponownie, może mieć ten sam tekst/font/color ale inny adres
- To powoduje duplicate tekstury w cache

**Scenariusz**:
1. FontManager::loadFont("path", 16) → font_ptr = 0x1000
2. TextureManager::createTextureFromText("Hello", font, color) → key = "Hello|0x1000|r,g,b,a"
3. FontManager cache jest czyszczony somehow (nie ma takiej metody, ale...)
4. FontManager::loadFont("path", 16) → font_ptr = 0x2000 (nowy adres)
5. TextureManager::createTextureFromText("Hello", font, color) → key = "Hello|0x2000|r,g,b,a"
6. **Duplikat tekstury!**

**Rekomendacja**: Używać (font_path, font_size) jako klucza fontu:
```cpp
// Potrzebna metoda w FontManager:
std::string getFontKey(const SharedFont& font) const;  // Zwraca "path|size"
// Lub w TextureManager:
// Przechowywać font_key (path|size) jako część cacheKey
```

### 3.3 ❌ TextInput cursor blinking wymusza recreate cache

```cpp
// text_input.cpp:218-220
if (hasKeyboardFocus()) {
    markDirty(); // Keep dirty for cursor blinking
}
```

**Problem**: Każda klatka gdy TextInput ma fokus → `markDirty()` → `renderToCache()` → nowa tekstura bufora.

**Implikacje**:
- Tekstura bufora elementu jest tworzone/usuwanie każdej klatki (500ms blink cycle)
- Nie jest to problem TextureManager, ale cache elementu

**Rekomendacja**:
```cpp
// TextInput powinien używać wantsDirectRender() = true dla cursor
// Lub oddzielny render cursor bez cache:
void TextInput::draw(SDL_Renderer* renderer) {
    drawBackgroundAndBorder(renderer);
    drawText(renderer);  // Na cache
    // Cursor rysowany bezpośrednie - poza cache elementu
}

// Lub użyć timestamp dla cursor blink bez markDirty():
if (SDL_GetTicks() - m_cursor_blink_time > 500) {
    m_show_cursor = !m_show_cursor;
    m_cursor_blink_time = SDL_GetTicks();
    // Nie markDirty() - cursor rysowany w drawDirect()
}
```

### 3.4 ⚠️ TextArea::m_line_textures.clear() nie usuwa z TextureManager

```cpp
// text_area.cpp:230-231
void TextArea::refreshTextures() {
    m_line_textures.clear();  // Czyści wektor SharedTexture
    // Ale tekstury pozostają w TextureManager::m_textureCache!
}
```

**Problem**: `SharedTexture` w wektorze jest czyści, ale `shared_ptr` w TextureManager cache pozostaje.

**Implikacje**:
- Po zmianie tekstu TextArea, stare tekstury linii pozostają w cache
- Jeśli TextArea jest często edytowana → cache rozrasta się

**Rekomendacja**:
```cpp
void TextArea::refreshTextures() {
    // Przed clear, oznacz tekstury jako "unused" somehow
    m_line_textures.clear();
    
    // TextureManager::pruneUnused() powinno być wywołane okresowo
}
```

### 3.5 ✅ FontManager - poprawna implementacja

FontManager jest dobrze zaimplementowany:

- **Transparent comparator** (`FontCacheKeyCompare`) - pozwala na wyszukiwanie `std::pair<std::string_view, int>` bez tworzenia `std::string`
- **FontKey** jako `(path, size)` - stabilny klucz, nie używa adresów wskaźników
- **loadDefaultFont()** - przechowuje domyślny font jako `SharedFont`

**Status**: ✅ POPRAWNE - brak problemów

---

## 4. Proponowane rozwiązania

### 4.1 TextureManager::pruneUnused()

```cpp
void TextureManager::pruneUnused() {
    auto it = m_textureCache.begin();
    while (it != m_textureCache.end()) {
        if (it->second.use_count() == 1) {  // Only TextureManager holds reference
            it = m_textureCache.erase(it);
        } else {
            ++it;
        }
    }
}
```

**Użycie**:
- Wywołać okresowo (np. co N klatek) lub po dużych zmianach UI
- TextArea może wywołać po `refreshTextures()`

### 4.2 Font path/size jako część klucza tekstury

```cpp
// FontManager potrzebuje:
struct FontInfo {
    std::string path;
    int size;
};

std::optional<FontInfo> TextureManager::getFontInfo(const SharedFont& font) const;

// TextureManager::createTextureFromText():
std::string cacheKey = std::string(text) + "|" + fontInfo.path + "|" + 
    std::to_string(fontInfo.size) + "|" + colorKey;
```

### 4.3 TextInput - cursor bez recreate cache

```cpp
class TextInput : public GUIElement {
    [[nodiscard]] bool wantsDirectRender() const override { return hasKeyboardFocus(); }
    
    void drawDirect(SDL_Renderer* renderer) override {
        // Rysuj cursor bezpośrednie
        if (m_show_cursor) {
            SDL_RenderFillRect(renderer, &cursor_rect);
        }
    }
    
    void draw(SDL_Renderer* renderer) override {
        // Rysuj text, background, border - na cache
        // Cursor rysowany w drawDirect() gdy hasKeyboardFocus()
    }
};
```

### 4.4 TextArea - lokalne tekstury bez TextureManager cache

```cpp
class TextArea : public GUIElement {
    // Zamiast używać TextureManager::createTextureFromText():
    void refreshTextures() {
        m_line_textures.clear();
        for (const auto& line : m_lines) {
            if (line.empty()) {
                m_line_textures.push_back(nullptr);
            } else {
                // Tworzy teksturę bezpośrednie, bez cache'owania w TextureManager
                SDL_Surface* surface = TTF_RenderUTF8_Blended(font.get(), line.c_str(), color);
                SDL_Texture* texture = SDL_CreateTextureFromSurface(m_manager.getRenderer(), surface);
                SDL_FreeSurface(surface);
                m_line_textures.push_back(SharedTexture(texture, SDLTextureDeleter()));
            }
        }
    }
};
```

**Zaleta**: Tekstury linii są zarządzane lokalnie, nie rozrastają TextureManager cache.

### 4.5 StringGrid - cache komórek lokalnie

```cpp
class StringGrid : public Panel {
    // Cache tekstur komórek lokalnie, nie w TextureManager
    std::map<std::string, SharedTexture> m_cellTextureCache;
    
    void drawCell(...) {
        std::string key = m_data[row][col];
        auto it = m_cellTextureCache.find(key);
        if (it == m_cellTextureCache.end()) {
            // Tworzy teksturę lokalnie
            auto texture = createLocalTexture(...);
            m_cellTextureCache[key] = texture;
        }
        SDL_RenderCopy(renderer, m_cellTextureCache[key].get(), nullptr, &destRect);
    }
    
    void clearCellCache() {
        m_cellTextureCache.clear();
    }
};
```

---

## 5. Podsumowanie

| Komponent | Status | Problem |
|-----------|--------|---------|
| **TextureManager** | ⚠️ | Brak cleanup/prune mechanizm |
| **FontManager** | ✅ | Poprawna implementacja |
| **Label** | ✅ | Poprawne użycie cache |
| **Button/Panel** | ✅ | Poprawne użycie Style.texture |
| **Checkbox/RadioButton** | ✅ | Poprawne użycie Style.texture |
| **TextInput** | ❌ | Cursor blinking wymusza recreate cache każdej klatki |
| **TextArea** | ❌ | Tekstury linii pozostają w TextureManager cache bez limitu |
| **ComboBox** | ✅ | Poprawne użycie cache |
| **StringGrid** | ⚠️ | Cache rozrasta się bez limitu dla dużych siatek |

### Priorytety naprawy:

1. **HIGH**: TextureManager::pruneUnused() - zapobiega memory leak
2. **HIGH**: TextInput cursor blinking - zapobiega recreate cache każdej klatki  
3. **MEDIUM**: Font pointer jako klucz - może powodować duplikaty w edge cases
4. **LOW**: StringGrid cache - optymalizacja dla bardzo dużych siatek

---

## 6. Diagram przepływu tekstur

```
┌─────────────────┐     ┌──────────────────┐     ┌─────────────┐
│   Widget draw() │────▶│ TextureManager   │────▶│ SDL_Texture │
│                 │     │ .createTexture   │     │ (GPU memory)│
│  "Hello"        │     │ FromText()       │     │             │
│  font, color    │     │                  │     │             │
└─────────────────┘     └──────────────────┘     └─────────────┘
                               │
                               ▼
                        ┌──────────────┐
                        │ m_textureCache│
                        │ (map<string,  │
                        │  SharedTexture)│
                        │              │
                        │ "Hello|ptr|   │
                        │  r,g,b,a" → tex│
                        └──────────────┘
                               │
                               ▼ (use_count == 1)
                        ❌ BRAK CLEANUP!
                        Tekstura pozostaje na zawsze
                        
═════════════════════════════════════════════════════════════

┌─────────────────┐     ┌──────────────────┐     ┌─────────────┐
│  GUIElement     │────▶│ renderToCache()  │────▶│CachedTexture│
│  m_isDirty=true │     │                  │     │(unique_ptr) │
│                 │     │ SDL_CreateTexture│     │             │
│                 │     │ (element size)   │     │ Per-element │
└─────────────────┘     └──────────────────┘     └─────────────┘
                               │
                               │ (TextInput hasKeyboardFocus)
                               ▼
                        markDirty() KAŻDA KLATKA
                        ❌ recreate cache każdej klatki!
```

---

## 7. Testy weryfikacyjne

```cpp
// test_texture_manager.cpp - proponowane testy

TEST_CASE("TextureManager cache grows without limit") {
    TextureManager tm(renderer);
    
    // Symulacja TextArea z dynamicznym tekstem
    for (int i = 0; i < 1000; ++i) {
        tm.createTextureFromText("Line_" + std::to_string(i), font, color);
    }
    
    CHECK(tm.getCacheSize() == 1000);  // 1000 tekstur w cache!
    
    // After "clearing" TextArea:
    // Tekstury pozostają w cache - MEMORY LEAK!
}

TEST_CASE("Font pointer as key causes duplicates") {
    FontManager fm;
    TextureManager tm(renderer);
    
    auto font1 = fm.loadFont("path.ttf", 16);
    auto text1 = tm.createTextureFromText("Hello", font1, color);
    
    // Symulacja font reload (nie ma takiej metody, ale...)
    fm.clearCache();  // Hypothetical
    auto font2 = fm.loadFont("path.ttf", 16);  // Nowy adres!
    auto text2 = tm.createTextureFromText("Hello", font2, color);
    
    CHECK(text1.get() != text2.get());  // Duplicate texture!
}

TEST_CASE("pruneUnused removes unreferenced textures") {
    TextureManager tm(renderer);
    
    auto tex1 = tm.createTextureFromText("A", font, color);  // use_count = 1
    auto tex2 = tm.createTextureFromText("B", font, color);  // use_count = 2 (tex2 holds ref)
    
    tex1.reset();  // Release reference
    
    tm.pruneUnused();
    
    CHECK(!tm.hasTexture("A|..."));  // Removed
    CHECK(tm.hasTexture("B|..."));   // Still exists (tex2 holds ref)
}
```