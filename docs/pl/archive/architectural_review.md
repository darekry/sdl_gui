# Przegląd Architektoniczny Biblioteki GUI
[Ta strona jest dostępna po angielsku](../../en/archive/architectural_review.md)
[Powrót do Archiwum](./README.md)

Data: 2025-07-06
## Wstęp

Niniejszy dokument podsumowuje wyniki przeglądu architektonicznego biblioteki. Celem analizy było zidentyfikowanie niespójności w kodzie w stosunku do założeń opisanych w `architecture.md`, a także wskazanie potencjalnych problemów ze skalowalnością i zaproponowanie rozwiązań.

## 1. Zidentyfikowane Niespójności Architektoniczne

### 1.1. Użycie Surowych Wskaźników do Zarządzania Dziećmi

**Problem:**
Komponenty `ComboBox` i `TabControl` przechowują surowe wskaźniki do swoich elementów podrzędnych (np. `Button* m_main_button`, `std::vector<Panel*> m_tabPanels`). Własność tych obiektów jest już zarządzana przez `std::unique_ptr` w kontenerze `m_children` klasy bazowej `GUIElement`.

**Ryzyko:**
- **Wiszące wskaźniki (Dangling Pointers):** Jeśli element-dziecko zostanie usunięty lub podmieniony, surowy wskaźnik nie zostanie zaktualizowany, co prowadzi do niezdefiniowanego zachowania.
- **Naruszenie zasady pojedynczej własności:** Architektura oparta na inteligentnych wskaźnikach zakłada jednego właściciela zasobu. Utrzymywanie dodatkowych, surowych wskaźników komplikuje zarządzanie cyklem życia obiektów.

**Rekomendacja:**
Należy usunąć surowe wskaźniki z tych klas. Dostęp do dzieci powinien odbywać się poprzez iterację po kontenerze `m_children` i, w razie potrzeby, użycie `dynamic_cast` do uzyskania wskaźnika na konkretny typ pochodny.

```cpp
// Przykład uzyskania dostępu do przycisku w ComboBox
// Zamiast: m_main_button->setLabel(...)
// Użyć:
if (!m_children.empty()) {
    if (auto main_button = dynamic_cast<Button*>(m_children[0].get())) {
        main_button->setLabel(...);
    }
}
```
### 1.2. Redundantny Parametr w Metodzie `render`

**Problem:**
Wszystkie metody `render()` w hierarchii dziedziczenia, począwszy od `GUIManager`, przyjmują jako argument `SDL_Renderer* renderer`. Jest to niezgodne z założeniem, że `GUIManager` jest centralnym dostawcą kontekstu, a każdy `GUIElement` ma do niego dostęp poprzez pole `m_manager`.

**Rekomendacja:**
Należy usunąć parametr `SDL_Renderer*` ze wszystkich sygnatur metod `render`. Implementacje powinny pobierać renderer bezpośrednio z menedżera:

```cpp
// Wewnątrz dowolnej metody render() w klasie pochodnej GUIElement
void MyElement::render() { // Bez parametru
    SDL_Renderer* renderer = m_manager.getRenderer();
    // ... logika renderowania ...
}
```

### 1.3. Zaszyte na Stałe Ścieżki do Zasobów

**Problem:**
W kilku miejscach, m.in. w `TextInput::updateTextTexture` i `GUIElement::setLabel`, ścieżka do pliku czcionki jest zaszyta na stałe w kodzie (`"assets/fonts/font.ttf"`).

**Ryzyko:**
- **Brak elastyczności:** Użytkownik biblioteki nie może w prosty sposób zmienić domyślnej czcionki ani struktury katalogów bez modyfikacji kodu źródłowego biblioteki.

**Rekomendacja:**
Należy korzystać z domyślnej czcionki ładowanej podczas inicjalizacji `FontManager`. Metody takie jak `setLabel` powinny używać `m_manager.getFontManager().getDefaultFont()`. Ścieżka do zasobów powinna być konfigurowalna w jednym miejscu – przy tworzeniu `GUIManager`.

### 1.4. Omijanie Menedżera Tekstur w Przykładach

**Problem:**
Kod w `examples/example_window.cpp` tworzy tekstury za pomocą własnej funkcji pomocniczej `createColorTexture` i ręcznie zarządza ich cyklem życia (`std::shared_ptr` z `SDL_DestroyTexture`), całkowicie omijając `TextureManager`.

**Rekomendacja:**
Należy rozszerzyć `TextureManager` o metodę `createColorTexture(SDL_Color color, int width, int height)`, która będzie odpowiedzialna za tworzenie jednokolorowych tekstur i zarządzanie nimi. Kod przykładów musi zostać zaktualizowany, aby korzystał z tej nowej funkcjonalności, co zapewni spójność architektoniczną.
## 2. Problemy ze Skalowalnością i Duplikacja Kodu

### 2.1. Powtarzalny Kod Inicjalizacji i Czyszczenia SDL w Przykładach

**Problem:**
Każdy plik w katalogu `examples/` zawiera długi i powtarzalny blok kodu odpowiedzialny za inicjalizację `SDL`, `SDL_image`, `SDL_ttf`, obsługę błędów i końcowe czyszczenie zasobów.

**Rekomendacja:**
Stworzyć dedykowaną klasę pomocniczą (np. `SDLApp`) działającą w oparciu o wzorzec RAII (Resource Acquisition Is Initialization). Konstruktor tej klasy byłby odpowiedzialny za całą inicjalizację, a destruktor za zwolnienie wszystkich zasobów. Klasa mogłaby również przechowywać `SDL_Window` i `SDL_Renderer`.

**Szkic implementacji:**
```cpp
// W nowym pliku, np. examples/helpers/sdl_app.hpp
class SDLApp {
public:
    SDLApp(const char* title, int w, int h);
    ~SDLApp();
    SDL_Renderer* getRenderer() { return renderer; }
    // ... inne gettery ...
private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
};

// W main()
int main(int, char**) {
    SDLApp app("Przykład", 800, 600);
    GUIManager gui(app.getRenderer());
    // ... logika GUI ...
    // pętla główna
    return 0; // Destruktor SDLApp automatycznie posprząta
}