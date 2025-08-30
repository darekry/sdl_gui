# Propozycja Systemu Animacji w Bibliotece GUI
[Ta strona jest dostępna po angielsku](../../en/proposals/animation_system_proposal.md)

[Powrót do Propozycji Funkcji](../feature_proposals.md)


## Wprowadzenie

Aby zaimplementować płynne, sterowane czasowo animacje (np. przejścia kolorów, ruch, zmiana rozmiaru), potrzebujemy dedykowanego systemu, który będzie działał w pętli aktualizacji i renderowania, modyfikując właściwości obiektów klatka po klatce. Obecny `TimerManager` jest zoptymalizowany pod zdarzenia jednorazowe lub cykliczne „co N ms”, ale nie pod ciągłe aktualizacje klatkowe wymagane przez animacje.

Niniejsza propozycja opisuje architekturę takiego systemu.

## Proponowane zmiany

### 1) Funkcje wygładzające (Easing)

Do uzyskania naturalnych, nieliniowych animacji potrzebujemy funkcji „easing”. 

- Akcja: dodać nowy plik nagłówkowy z kolekcją funkcji easing, np. `src/easing.hpp`.
- Zawartość: zestaw funkcji typu `linear`, `easeInQuad`, `easeOutQuad`, `easeInOutQuad` itp., przyjmujących postęp animacji `t` z zakresu [0.0, 1.0] i zwracających przekształcony postęp.

Docelowo ten plik będzie używany przez system animacji oraz przez użytkowników, którzy chcą własnych krzywych.

### 2) AnimationManager

„Serce” systemu — zarządza cyklem życia wszystkich aktywnych animacji.

- Akcja: utworzyć klasę `AnimationManager` w plikach: [`src/animation_manager.hpp`](../../src/animation_manager.hpp:24) i [`src/animation_manager.cpp`](../../src/animation_manager.cpp:1).
- Struktura `Animation` (przykładowo):
  - `std::function<void(float)> update_callback` — wywoływana w każdej klatce z postępem animacji.
  - `Uint32 start_time`, `Uint32 duration_ms` — czas startu i czas trwania animacji.
  - `std::function<float(float)> easing_function` — funkcja easing.
  - `bool is_finished` — flaga zakończenia.

- Logika `AnimationManager::update()`:
  1. Iteruje po aktywnych animacjach.
  2. Oblicza postęp `raw = (now - start_time) / duration_ms` (ograniczony do [0, 1]).
  3. Stosuje `easing_function(raw)`.
  4. Wywołuje `update_callback(progress)`.
  5. Oznacza zakończone animacje i usuwa je z listy.

Dodatkowo manager powinien oferować metody:
- dodawania animacji (zwracając uchwyt albo `id`),
- anulowania animacji (po `id`),
- anulowania wszystkich animacji powiązanych z danym elementem (jeśli powiązanie jest utrzymywane).

### 3) Integracja z GUIManager

`GUIManager` musi zarządzać instancją `AnimationManager`, aby animacje były aktualizowane w cyklu życia GUI.

- Zmiany w [`GUIManager`](../../src/gui_manager.hpp:19):
  - pole `std::unique_ptr<AnimationManager> m_animationManager;`
  - inicjalizacja w konstruktorze
  - w [`GUIManager::cleanup()`](../../src/gui_manager.cpp:14) wywołanie `m_animationManager->update()`
  - publiczny getter `AnimationManager* getAnimationManager()` (lub referencja).

Dzięki temu każdy `GUIElement` lub kod aplikacji może uruchamiać animacje przez globalny manager.

## Przykład użycia

Po implementacji, stworzenie animacji przejścia koloru (interpolacja) może wyglądać następująco:

```cpp
// Załóżmy, że mamy panel oraz dostęp do guiManager
SDL_Color start_color = { 40,  40,  40, 255};
SDL_Color end_color   = {200, 200, 255, 255};

// Referencja/uchwyt na panel (np. GUIElement pochodny)
Panel& panel_ref = *myPanel;

// Dodanie animacji trwającej 2000 ms z funkcją easeInOutQuad
guiManager.getAnimationManager()->addAnimation(
    // update_callback: progress w zakresie [0..1] po zastosowaniu easing
    [=, &panel_ref](float progress) {
        Uint8 r = static_cast<Uint8>(start_color.r + (end_color.r - start_color.r) * progress);
        Uint8 g = static_cast<Uint8>(start_color.g + (end_color.g - start_color.g) * progress);
        Uint8 b = static_cast<Uint8>(start_color.b + (end_color.b - start_color.b) * progress);
        panel_ref.setBackgroundColor({r, g, b, 255});
        panel_ref.markDirty(); // aby odświeżyć cache renderowania
    },
    2000, // duration_ms
    Easing::easeInOutQuad // funkcja easing
);
```

Z perspektywy API:
- `addAnimation` może przyjmować dodatkowo identyfikator właściciela (np. wskaźnik/`std::weak_ptr` do elementu), aby umożliwić bezpieczne anulowanie animacji przy usuwaniu elementu.
- Alternatywnie można zapewnić wersje `addAnimationFor(GUIElement*, ...)`, aby związać animację z cyklem życia elementu.

## Uwagi implementacyjne

- Bezpieczeństwo i cykl życia:
  - Preferowane jest wiązanie animacji z właścicielem (np. `GUIElement*`) tak, aby przy usunięciu elementu animacje były automatycznie anulowane.
  - W callbackach nie należy przechowywać surowych wskaźników długoterminowo — rekomendowane są mechanizmy walidacji (np. `weak_ptr` do właściciela).

- Wydajność:
  - `update()` powinna być lekka — iterować po krótkiej liście aktywnych animacji i usuwać zakończone w jednym przebiegu.
  - Unikać alokacji w pętli aktualizacji.

- Kompozycja:
  - `AnimationManager` nie powinien znać szczegółów renderingu. Steruje wyłącznie postępem i wywołuje callback użytkownika.

## Podsumowanie

- Wprowadzamy `AnimationManager` do obsługi płynnych animacji opartych o czas i funkcje „easing”.
- Integrujemy go z cyklem życia `GUIManager` (aktualizacja w `cleanup()`).
- Udostępniamy proste API do deklaratywnego tworzenia animacji dowolnych właściwości (kolor, pozycja, rozmiar itp.).
- Zapewniamy mechanizmy bezpieczeństwa (anulowanie przy niszczeniu elementów) i prostą rozbudowę (własne funkcje easing).