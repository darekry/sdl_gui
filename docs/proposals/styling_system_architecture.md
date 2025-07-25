# Architektura Systemu Stylizacji GUI

## 1. Wprowadzenie

Obecny system stylizacji w bibliotece jest niespójny i ograniczony. Ten dokument proponuje nową, rozszerzalną architekturę, która ujednolici wygląd i działanie komponentów, wprowadzi system motywów oraz uprości personalizację elementów interfejsu.

## 2. Kluczowe Struktury Danych

### 2.1. `ElementState` (Stan Elementu)

Wprowadzamy `enum class` do jawnego definiowania stanów, w jakich może znaleźć się element. Jest to fundament do zarządzania stylami zależnymi od interakcji użytkownika.

```cpp
enum class ElementState {
    Normal,
    Hover,
    Pressed,
    Disabled
};
```

### 2.2. `Style` (Styl)

Struktura `Style` będzie kontenerem na wszystkie atrybuty wizualne pojedynczego stanu elementu. Użycie `std::optional` pozwoli na dziedziczenie atrybutów z domyślnego motywu – jeśli atrybut w stylu elementu ma wartość `std::nullopt`, zostanie użyta wartość z motywu.

```cpp
#include "texture_manager.hpp" // Dla SharedTexture
#include <optional>
#include <SDL_pixels.h>

struct Style {
    std::optional<SDL_Color> backgroundColor;
    std::optional<SDL_Color> textColor;
    std::optional<SharedTexture> texture;
    std::optional<SDL_Color> borderColor;
    std::optional<int> borderWidth;
};
```

### 2.3. `Theme` (Motyw)

Klasa `Theme` będzie centralnym punktem przechowywania domyślnych stylów dla wszystkich typów komponentów. Pozwoli to na globalną zmianę wyglądu aplikacji. Domyślny motyw będzie imitował styl Windows 95/98.

Kluczowe jest użycie `std::string` jako klucza, co pozwoli na łatwe dodawanie stylów dla nowo tworzonych klas komponentów bez modyfikacji samej klasy `Theme`.

```cpp
#include <map>
#include <string>

class Theme {
public:
    // Ustawia domyślny styl dla danego typu komponentu i stanu
    void setStyle(const std::string& componentType, ElementState state, Style style);

    // Pobiera domyślny styl. Zwraca styl domyślny, jeśli nie ma specyficznego.
    const Style& getStyle(const std::string& componentType, ElementState state) const;
    
    // Metoda do tworzenia domyślnego motywu "Windows 95"
    static Theme createDefaultTheme();

private:
    // Mapa: Typ komponentu -> Mapa: Stan -> Styl
    std::map<std::string, std::map<ElementState, Style>> styles;
    Style defaultStyle; // Styl używany, gdy brakuje specyficznego
};
```

## 3. Zmiany w API

### 3.1. `GUIManager`

`GUIManager` będzie właścicielem obiektu `Theme` i udostępni go wszystkim elementom.

```cpp
class GUIManager {
public:
    // ... istniejące metody ...

    // Ustawia nowy motyw
    void setTheme(Theme theme);

    // Zwraca referencję do aktualnego motywu
    Theme& getTheme();

private:
    // ... istniejące pola ...
    Theme m_theme = Theme::createDefaultTheme(); // Domyślny motyw
};
```

### 3.2. `GUIElement`

`GUIElement` zostanie znacząco zmodyfikowany, aby zarządzać własnymi stylami i współpracować z systemem motywów.

```cpp
class GUIElement {
public:
    // ... istniejące metody ...

    // Ustawia styl dla konkretnego stanu
    void setStyle(ElementState state, Style style);

    // Pobiera styl dla danego stanu (może być pusty)
    const std::optional<Style>& getStyle(ElementState state) const;

    // Pobiera ostateczny, "rozwiązany" styl - łącząc własny styl ze stylem z motywu
    Style getResolvedStyle() const;

    // Metody pomocnicze do modyfikacji stylu dla konkretnego stanu
    void setBackgroundColor(ElementState state, SDL_Color color);
    void setTextColor(ElementState state, SDL_Color color);
    void setTexture(ElementState state, SharedTexture texture);
    void setBorder(ElementState state, SDL_Color color, int width);

    // Zwraca typ komponentu jako string (do implementacji w klasach pochodnych)
    virtual const char* getComponentType() const { return "GUIElement"; }

protected:
    // ... istniejące pola ...
    
    // Przechowuje style specyficzne dla tego elementu
    std::map<ElementState, Style> m_styles;

    // Aktualny stan elementu
    ElementState m_currentState = ElementState::Normal;

private:
    // Prywatna metoda pomocnicza do łączenia stylów
    Style resolveStyle(const Style& base, const std::optional<Style>& override) const;
};
```
**Ważna uwaga:** Każda klasa dziedzicząca po `GUIElement` (np. `Button`, `Checkbox`) będzie musiała nadpisać metodę `getComponentType()`, aby system motywów mógł poprawnie identyfikować komponenty:

```cpp
// W klasie Button
const char* getComponentType() const override { return "Button"; }
```

### 3.3. Usunięcie starego API

Metody takie jak `setNormalTexture`, `setHoverTexture` w `RadioButton` zostaną usunięte i zastąpione nowym, jednolitym API `setStyle` lub metodami pomocniczymi (`setTexture(ElementState::Hover, ...)`).

## 4. Logika Renderowania i Wyboru Stylu

Proces decyzyjny w metodzie `render()` elementu będzie wyglądał następująco:

```mermaid
graph TD
    A[Rozpoczęcie renderowania elementu] --> B{Pobierz aktualny stan<br>(m_currentState)};
    B --> C[Wywołaj getResolvedStyle()];
    C --> I[Użyj zwróconego stylu do<br>renderowania tła, ramki, tekstu];
    I --> J[Zakończ renderowanie];
```

**Implementacja `getResolvedStyle`**:

```cpp
Style GUIElement::getResolvedStyle() const {
    // 1. Pobierz styl domyślny z motywu
    const auto& themeStyle = m_manager.getTheme().getStyle(getComponentType(), m_currentState);
    
    // 2. Sprawdź, czy istnieje styl specyficzny dla tego elementu
    auto it = m_styles.find(m_currentState);
    if (it != m_styles.end()) {
        // 3. Połącz style - styl elementu nadpisuje domyślny
        return resolveStyle(themeStyle, it->second);
    } else {
        // Zwróć styl z motywu, jeśli brak lokalnego
        return themeStyle;
    }
}
```

## 5. Uzasadnienie Projektu i Rozważane Alternatywy

Proponowana architektura jest kompromisem między elastycznością, wydajnością i prostotą użycia. Poniżej wyjaśniono kluczowe decyzje projektowe.

*   **`std::optional` w `Style`**: Zapewnia czysty mechanizm "dziedziczenia" właściwości z motywu. Bez niego musielibyśmy używać wskaźników lub dodatkowych flag `bool`, co komplikowałoby kod. To rozwiązanie jest fundamentem dla nadpisywania stylów.

*   **Identyfikacja komponentu (`getComponentType`)**: Użycie wirtualnej funkcji i `std::string` jako klucza w motywie jest kluczowe dla **rozszerzalności**. Pozwala użytkownikom biblioteki na tworzenie własnych, niestandardowych komponentów i definiowanie dla nich domyślnych stylów bez modyfikacji samej biblioteki. Alternatywą byłoby użycie `enum`, co byłoby szybsze, ale całkowicie zamknęłoby system na nowe typy komponentów.

*   **Dynamiczne rozwiązywanie stylów (`getResolvedStyle`)**: Styl jest obliczany w momencie renderowania. Główną zaletą jest możliwość **dynamicznej zmiany motywu** w trakcie działania aplikacji – wszystkie elementy natychmiast przyjmą nowy wygląd.

### 5.1. Potencjalne Uproszczenie i jego Konsekwencje

Możemy uprościć system, ale kosztem elastyczności.

**Propozycja uproszczenia:** Zamiast obliczać styl dynamicznie, moglibyśmy **kopiować styl z motywu do elementu w momencie jego tworzenia**.

*   **Jak by to działało:** W konstruktorze `GUIElement` style z motywu byłyby kopiowane do lokalnej mapy `m_styles`. Metoda `getResolvedStyle` stawałaby się trywialna – po prostu zwracałaby styl z lokalnej mapy.
*   **Zalety:**
    *   **Wydajność:** Brak logiki łączenia stylów i wirtualnych wywołań w pętli renderowania.
    *   **Prostszy kod:** `getResolvedStyle` jest znacznie prostsze.
*   **Wady:**
    *   **Utrata dynamicznego przełączania motywów:** Jeśli motyw zostanie zmieniony po utworzeniu elementów, nie zaktualizują one swojego wyglądu. To kluczowa strata funkcjonalności.
    *   **Większe zużycie pamięci:** Każdy element przechowuje pełny zestaw stylów, a nie tylko te nadpisane.

Biorąc pod uwagę cele biblioteki (elastyczność i rozszerzalność), obecny, dynamiczny model wydaje się lepszym wyborem.

## 6. Przykład Użycia API

Poniższy kod pokazuje, jak deweloper mógłby dostosować wygląd przycisku, nadpisując domyślne ustawienia z motywu.

```cpp
// Inicjalizacja menedżera
GUIManager guiManager(renderer);

// Tworzenie przycisku
auto myButton = std::make_unique<Button>(guiManager, 50, 50, 150, 40);

// --- Personalizacja ---

// 1. Zmiana tła dla stanu Normal
myButton->setBackgroundColor(ElementState::Normal, {200, 200, 255, 255}); // Jasnoniebieski

// 2. Ustawienie tekstury i innego koloru tekstu dla stanu Hover
Style hoverStyle;
hoverStyle.texture = textureManager.load("assets/button_hover.png");
hoverStyle.textColor = SDL_Color{255, 255, 0, 255}; // Żółty tekst
myButton->setStyle(ElementState::Hover, hoverStyle);

// 3. Czerwona ramka dla stanu Pressed
myButton->setBorder(ElementState::Pressed, {255, 0, 0, 255}, 2);

// Dodanie przycisku do menedżera
guiManager.addElement(std::move(myButton));