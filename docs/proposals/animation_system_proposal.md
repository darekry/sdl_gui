# Propozycja Systemu Animacji w Bibliotece GUI

## Wprowadzenie

Aby zaimplementować płynne, sterowane czasowo animacje (np. przejścia kolorów, ruch, zmiana rozmiaru), potrzebujemy dedykowanego systemu, który będzie działał równolegle do pętli renderowania i aktualizował właściwości obiektów klatka po klatce. Obecny `TimerManager` jest przeznaczony do jednorazowych lub cyklicznych zdarzeń, a nie do ciągłych aktualizacji klatkowych.

Poniższa propozycja opisuje architekturę takiego systemu.

## Proponowane Zmiany

### 1. Funkcje "Easing" (Wygładzania)

Do uzyskania naturalnych, nieliniowych animacji, potrzebujemy funkcji wygładzających.

*   **Akcja:** Stworzyć nowy plik `src/easing.hpp`.
*   **Zawartość:** Plik będzie zawierał kolekcję statycznych funkcji, takich jak `linear`, `easeInQuad`, `easeOutQuad`, `easeInOutQuad`, które przyjmują postęp animacji (0.0 do 1.0) i zwracają zmodyfikowaną wartość.

### 2. `AnimationManager`

Serce systemu, odpowiedzialne za zarządzanie cyklem życia wszystkich aktywnych animacji.

*   **Akcja:** Stworzyć nową klasę `AnimationManager` w plikach `src/animation_manager.hpp` i `src/animation_manager.cpp`.
*   **Struktura `Animation`:** Wewnątrz managera zdefiniować strukturę `Animation`, przechowującą:
    *   `std::function<void(float)> update_callback`: Funkcja wywoływana w każdej klatce z postępem animacji.
    *   `Uint32 start_time`, `Uint32 duration_ms`: Czas rozpoczęcia i trwania animacji.
    *   `std::function<float(float)> easing_function`: Wskaźnik na funkcję easingu.
    *   `bool is_finished`: Flaga oznaczająca zakończenie animacji.
*   **Logika `AnimationManager::update()`:**
    1.  Iteruje po wszystkich aktywnych animacjach.
    2.  Oblicza procentowy postęp (`(currentTime - startTime) / duration`).
    3.  Stosuje funkcję easingu do postępu.
    4.  Wywołuje `update_callback` z przetworzonym postępem.
    5.  Oznacza zakończone animacje i usuwa je z listy.

### 3. Integracja z `GUIManager`

`GUIManager` musi zarządzać instancją `AnimationManager`.

*   **Akcja:** Zmodyfikować klasę `GUIManager`.
*   **Zmiany:**
    *   Dodać `std::unique_ptr<AnimationManager> animation_manager;`.
    *   Zainicjalizować go w konstruktorze `GUIManager`.
    *   W metodzie `GUIManager::cleanup()` dodać wywołanie `animation_manager->update()`, aby zintegrować system animacji z główną pętlą aplikacji.
    *   Dodać publiczny getter `getAnimationManager()`, aby umożliwić dostęp z zewnątrz.

## Przykład Użycia

Po implementacji, stworzenie animacji przejścia koloru w kodzie aplikacji wyglądałoby następująco:

```cpp
checkbox->onToggle = [&](bool is_checked) {
    SDL_Color start_color = ...;
    SDL_Color end_color = ...;
    
    guiManager.getAnimationManager()->addAnimation(
        [=, &panel_ref](float progress) {
            // Interpolacja kolorów
            Uint8 r = start_color.r + (end_color.r - start_color.r) * progress;
            Uint8 g = start_color.g + (end_color.g - start_color.g) * progress;
            Uint8 b = start_color.b + (end_color.b - start_color.b) * progress;
            panel_ref.setBackgroundColor({r, g, b, 255});
        }, 
        2000, 
        Easing::easeInOutQuad
    );
};