# Propozycje Nowych Funkcji dla Biblioteki GUI

Poniższa lista zawiera zebrane propozycje nowych funkcji i ulepszeń dla biblioteki, zarówno te sugerowane przez użytkownika, jak i dodatkowe pomysły.

## Propozycje Użytkownika

1.  **Obsługa wklejania tekstu (Clipboard)**: Umożliwienie wklejania tekstu do kontrolek typu `TextInput` i `TextArea` za pomocą `SDL_SetClipboardText()` i `SDL_GetClipboardText()`.
2.  **Menu kontekstowe**: Widget pojawiający się po kliknięciu prawym przyciskiem myszy na elemencie, który ma zdefiniowane takie menu. Powinien być w pełni konfigurowalny i obsługiwać akcje dla swoich pozycji. **[ZAIMPLEMENTOWANE]**
3.  **Combobox "do góry"**: Rozszerzenie `Combobox`, aby automatycznie otwierał listę opcji w górę, jeśli brakuje mu miejsca pod spodem w kontenerze-rodzicu.

## Propozycje Dodatkowe

1.  **Managery Układu (Layout Managers)**:
    *   **Stack Layout**: Układa elementy wertykalnie lub horyzontalnie, automatycznie zarządzając ich pozycjami.
    *   **Grid Layout**: Organizuje elementy w siatce o definiowalnych wierszach i kolumnach.
    *   Upraszcza to tworzenie złożonych interfejsów bez potrzeby ręcznego liczenia koordynatów.

2.  **Podpowiedzi (Tooltips)**: Małe okienka z tekstem informacyjnym pojawiające się, gdy kursor myszy najedzie na dany element i zatrzyma się na chwilę. **[ZAIMPLEMENTOWANE]**

3.  **Dialog wyboru pliku/katalogu**: Standardowy, wbudowany widget do przeglądania i wybierania plików lub folderów z systemu plików.

4.  **Wiązanie danych (Data Binding)**: Mechanizm pozwalający na powiązanie właściwości widgetu (np. tekst w `TextInput`, stan `Checkbox`) bezpośrednio ze zmiennymi w kodzie aplikacji. Zmiana w UI automatycznie aktualizuje zmienną i odwrotnie.

5.  **Proste animacje/przejścia**: Wsparcie dla płynnych przejść, np. animowane pojawianie/znikanie (fade-in/fade-out) okien lub subtelne zmiany kolorów po najechaniu myszą, aby uczynić interfejs bardziej "żywym".


### 2. Rekomendowane Rozwiązanie Alternatywne: Wewnętrzny Menedżer Zdarzeń Czasowych **[ZAIMPLEMENTOWANE - jako TimerManager]**

-   **Opis**: Stworzenie dedykowanej klasy, np. `TimeEventManager`, zintegrowanej z `GUIManager`. Menedżer ten utrzymuje kolejkę zaplanowanych zdarzeń. W każdej klatce pętli głównej aplikacji `GUIManager` wywołuje metodę `TimeEventManager::update(deltaTime)`, gdzie `deltaTime` to czas, jaki upłynął od ostatniej klatki. Menedżer sprawdza, które zdarzenia powinny zostać uruchomione i wykonuje przypisane do nich akcje (callbacki) bezpośrednio w głównym wątku.
-   **Zalety**:
    *   **Pełna kontrola i bezpieczeństwo**: Wszystkie operacje (dodawanie, aktualizacja, wykonywanie zdarzeń) odbywają się w jednym wątku (głównym), co eliminuje całkowicie problemy związane z wielowątkowością i synchronizacją.
    *   **Bezpieczne zarządzanie cyklem życia**: Ponieważ zdarzenia są powiązane z elementami GUI zarządzanymi przez `GUIManager`, można bezpiecznie używać wskaźników lub `std::weak_ptr` do powiązania akcji z elementem. Jeśli element zostanie usunięty, można łatwo anulować powiązane z nim zdarzenia czasowe.
    *   **Elastyczność**: System można łatwo rozbudować o wsparcie dla powtarzających się zdarzeń, animacji opartych na klatkach (a nie tylko na czasie), a także grupowania i anulowania zdarzeń.
    *   **Niski narzut**: Implementacja jest prosta i nie wymaga skomplikowanych mechanizmów synchronizacji.

### Porównanie i Rekomendacja

**Rekomendacja:** Zdecydowanie zaleca się implementację **wewnętrznego menedżera zdarzeń czasowych**. Takie podejście jest znacznie bezpieczniejsze, prostsze w utrzymaniu i bardziej elastyczne. Idealnie wpisuje się w istniejącą architekturę opartą na `GUIManager` jako centralnym punkcie zarządzania, zapewniając spójność i unikając typowych pułapek wielowątkowości.

```mermaid
sequenceDiagram
    participant App as Aplikacja
    participant GM as GUIManager
    participant TEM as TimeEventManager
    participant Element as GUIElement

    Note over App: Pętla główna aplikacji
    App->>App: Oblicz deltaTime
    App->>GM: update(deltaTime)
    GM->>TEM: update(deltaTime)

    Note over TEM: Sprawdź, czy nadszedł czas na jakieś zdarzenie
    alt Zdarzenie do wykonania
        TEM->>Element: wykonaj_akcje()
        Note over Element: np. pokaż tooltip, zmień kolor
    end

    Note over App: Koniec aktualizacji, początek renderowania
    App->>GM: render()
