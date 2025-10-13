# Propozycje koncepcji narzędzia do budowy GUI

Ten dokument przedstawia trzy propozycje koncepcji narzędzia do wizualnego planowania interfejsu użytkownika dla biblioteki SDL_GUI. Propozycje są ułożone od najprostszej do najbardziej zaawansowanej, uwzględniając obecną architekturę biblioteki.

---

## Koncepcja 1: SDL_GUI Markup Language (SGML)

### Nazwa i krótkie podsumowanie

**SGML (SDL_GUI Markup Language)** to proste, deklaratywne podejście oparte na formacie XML lub JSON. Deweloper definiuje strukturę i właściwości interfejsu w pliku tekstowym, który jest następnie parsowany w czasie rzeczywistym przez dedykowaną klasę w bibliotece w celu dynamicznego tworzenia obiektów GUI.

### Szczegółowy opis

Koncepcja zakłada rozszerzenie biblioteki o parser, który potrafiłby odczytać plik `.sgml` (w formacie XML) lub `.sgml.json`. Parser rekurencyjnie przechodziłby przez strukturę pliku, tworząc odpowiednie instancje widgetów (`Button`, `Panel`, `Window` itd.) i ustawiając ich właściwości na podstawie atrybutów (np. `x`, `y`, `width`, `height`, `text`). Hierarchia w pliku XML/JSON bezpośrednio odpowiadałaby hierarchii rodzic-dziecko w `GUIManager`.

**Technologie:**
*   **C++:** Implementacja parsera jako nowej klasy, np. `GUILoader`.
*   **Biblioteka do parsowania XML/JSON:** Można użyć lekkiej, zewnętrznej biblioteki C++ typu "header-only", takiej jak `tinyxml2` dla XML lub `nlohmann/json` dla JSON, aby uniknąć komplikowania procesu budowania.

### Zalety i wady

**Zalety:**
*   **Prostota implementacji:** Wymaga jedynie dodania logiki parsowania, bez tworzenia osobnego narzędzia graficznego.
*   **Separacja widoku od logiki:** Oddziela definicję interfejsu od kodu C++, co ułatwia zarządzanie i modyfikacje.
*   **Szybkie prototypowanie:** Zmiany w wyglądzie interfejsu nie wymagają rekompilacji kodu C++, wystarczy edytować plik tekstowy.
*   **Łatwość integracji:** Dobrze komponuje się z istniejącą architekturą opartą na `GUIElement` i `GUIManager`.

**Wady:**
*   **Brak wizualizacji:** Deweloper musi wyobrazić sobie wygląd interfejsu na podstawie kodu, co jest podatne na błędy.
*   **Ręczne pisanie:** Ręczne tworzenie plików XML/JSON może być czasochłonne i nużące przy bardziej złożonych interfejsach.
*   **Ograniczone możliwości:** Trudniejsze do zaimplementowania byłoby bindowanie zdarzeń (callbacków) bezpośrednio w pliku markup. Wymagałoby to np. systemu identyfikatorów i ręcznego przypisywania funkcji w kodzie C++.

### Przykład użycia (XML)

```xml
<!-- main_window.sgml -->
<Window id="mainWindow" x="100" y="100" width="400" height="300" title="Moje Okno">
  <Panel id="mainPanel" x="10" y="10" width="380" height="280">
    <Label text="Witaj w SDL_GUI!" x="20" y="20" width="340" height="30" />
    <Button id="okButton" text="OK" x="150" y="240" width="80" height="30" />
  </Panel>
</Window>
```

---

## Koncepcja 2: SDL_GUI Builder Lite

### Nazwa i krótkie podsumowanie

**SDL_GUI Builder Lite** to samodzielna, lekka aplikacja desktopowa, która pozwala na wizualne projektowanie interfejsu metodą "przeciągnij i upuść". Narzędzie generuje plik w formacie **SGML** (z Koncepcji 1), który następnie może być wczytany przez aplikację docelową.

### Szczegółowy opis

Aplikacja byłaby zbudowana przy użyciu samej biblioteki SDL_GUI. Oferowałaby prosty interfejs składający się z:
1.  **Palety widgetów:** Lista dostępnych komponentów (`Button`, `Label`, `Panel` itp.), które można przeciągnąć na obszar roboczy.
2.  **Obszaru roboczego:** Wizualna reprezentacja okna aplikacji, gdzie umieszcza się i aranżuje widgety.
3.  **Inspektora właściwości:** Panel, w którym można edytować właściwości zaznaczonego widgetu (pozycja, rozmiar, tekst, kolory, identyfikator).

Po zakończeniu projektowania, użytkownik zapisuje projekt, a narzędzie generuje plik `.sgml` lub `.sgml.json`.

**Technologie:**
*   **C++ i SDL_GUI:** Narzędzie byłoby "samohostujące" – napisane przy użyciu biblioteki, którą wspomaga.
*   **System serializacji:** Wykorzystanie tej samej biblioteki do parsowania/zapisu XML/JSON co w Koncepcji 1.

### Zalety i wady

**Zalety:**
*   **Wizualne projektowanie:** Znacznie przyspiesza i ułatwia tworzenie interfejsów w porównaniu do ręcznego pisania XML/JSON.
*   **Niski próg wejścia:** Intuicyjna obsługa dla osób niezaznajomionych ze składnią SGML.
*   **Wciąż zachowuje separację:** Generowany plik nadal oddziela definicję UI od logiki aplikacji.
*   **Realistyczny podgląd:** Ponieważ narzędzie używa tej samej biblioteki, podgląd interfejsu jest bardzo zbliżony do finalnego wyglądu.

**Wady:**
*   **Większy nakład pracy:** Wymaga stworzenia i utrzymania osobnej aplikacji.
*   **Problem "kurczaka i jajka":** Budowanie narzędzia w SDL_GUI wymagałoby, aby biblioteka była już na tyle dojrzała, by udźwignąć interfejs samego buildera.
*   **Synchronizacja:** Wszelkie nowe widgety dodane do biblioteki SDL_GUI musiałyby być również dodane do palety komponentów w Builderze.

### Przykład użycia

Użytkownik wizualnie układa komponenty, a narzędzie generuje taki sam plik XML/JSON jak w Koncepcji 1. Proces jest wizualny, a efekt końcowy (plik) jest taki sam.

---

## Koncepcja 3: Zintegrowany Edytor Wizualny (WYSIWYG)

### Nazwa i krótkie podsumowanie

**Zintegrowany Edytor Wizualny (WYSIWYG - What You See Is What You Get)** to najbardziej zaawansowane podejście. Byłoby to narzędzie głęboko zintegrowane ze środowiskiem deweloperskim (np. jako plugin do VS Code) lub samodzielna, potężna aplikacja, która nie tylko pozwala na wizualne projektowanie, ale także **generuje kod C++**.

### Szczegółowy opis

To narzędzie działałoby podobnie do znanych edytorów GUI (np. Qt Designer, Interface Builder w Xcode). Oprócz funkcji z Koncepcji 2, oferowałoby:
1.  **Generowanie kodu C++:** Zamiast (lub oprócz) pliku SGML, edytor generowałby plik nagłówkowy (`.hpp`) z deklaracjami wskaźników do widgetów oraz plik źródłowy (`.cpp`) z kodem inicjalizującym te widgety i dodającym je do `GUIManager`.
2.  **System sygnałów i slotów:** Umożliwiałby wizualne łączenie zdarzeń (np. `onClick` przycisku) z nazwami funkcji (slotami), które deweloper musiałby następnie zaimplementować w swoim kodzie.
3.  **Podgląd na żywo:** Możliwość uruchomienia zaprojektowanego okna w trybie podglądu bez opuszczania edytora.
4.  **Szablony i reużywalne komponenty:** Możliwość tworzenia własnych, złożonych komponentów i zapisywania ich jako szablonów.

**Technologie:**
*   **C++ i SDL_GUI:** Dla logiki i renderowania podglądu.
*   **Framework GUI dla samego narzędzia:** Prawdopodobnie coś bardziej dojrzałego niż SDL_GUI, np. Qt lub ImGui, aby stworzyć rozbudowany interfejs edytora. Alternatywnie, stworzenie go jako rozszerzenia do VS Code przy użyciu technologii webowych (TypeScript, Webview API).
*   **System szablonów kodu:** Narzędzie do generowania kodu C++ na podstawie szablonów (np. `inja`, `mustache`).

### Zalety i wady

**Zalety:**
*   **Najwyższa produktywność:** Najszybszy sposób na tworzenie i iterowanie po skomplikowanych interfejsach.
*   **Pełna integracja z kodem:** Generowanie kodu C++ eliminuje potrzebę ręcznego tworzenia obiektów i zarządzania nimi.
*   **Zarządzanie zdarzeniami:** Wizualne bindowanie zdarzeń upraszcza logikę aplikacji.
*   **Profesjonalne narzędzie:** Stawia bibliotekę SDL_GUI na równi z bardziej dojrzałymi frameworkami.

**Wady:**
*   **Bardzo wysoka złożoność implementacji:** To duży, osobny projekt programistyczny.
*   **Ryzyko "sztywnego" kodu:** Automatycznie generowany kod może być trudny do modyfikacji i integracji z istniejącą, niestandardową architekturą.
*   **Ścisłe powiązanie:** Zmiany w API biblioteki SDL_GUI wymagałyby natychmiastowych i potencjalnie skomplikowanych aktualizacji w edytorze.
*   **Utrzymanie:** Wymaga ogromnego nakładu pracy w zakresie utrzymania i rozwoju.

### Przykład użycia (wygenerowany kod C++)

Użytkownik projektuje interfejs, a narzędzie generuje pliki:

**`main_window_ui.hpp`**
```cpp
#pragma once
#include "gui_manager.hpp"
#include "window.hpp"
#include "panel.hpp"
#include "label.hpp"
#include "button.hpp"

class MainWindowUI {
public:
    Window* mainWindow = nullptr;
    Panel* mainPanel = nullptr;
    Label* welcomeLabel = nullptr;
    Button* okButton = nullptr;

    void setupUI(GUIManager& manager) {
        // Kod inicjalizujący zostanie wygenerowany w pliku .cpp
    }
};
```

**`main_window_ui.cpp`**
```cpp
#include "main_window_ui.hpp"

void MainWindowUI::setupUI(GUIManager& manager) {
    mainWindow = new Window(manager, 100, 100, 400, 300, "Moje Okno");
    mainPanel = new Panel(manager, 10, 10, 380, 280);
    welcomeLabel = new Label(manager, 20, 20, 340, 30, "Witaj w SDL_GUI!");
    okButton = new Button(manager, 150, 240, 80, 30, "OK");

    mainPanel->addChild(std::unique_ptr<GUIElement>(welcomeLabel));
    mainPanel->addChild(std::unique_ptr<GUIElement>(okButton));
    mainWindow->addChild(std::unique_ptr<GUIElement>(mainPanel));
    manager.addElement(std::unique_ptr<GUIElement>(mainWindow));
}