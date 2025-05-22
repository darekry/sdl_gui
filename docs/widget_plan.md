# Plan Poprawy Spójności i Zastosowania Najlepszych Praktyk w Bibliotece GUI SDL

**Cel:** Zwiększenie spójności, czytelności i bezpieczeństwa kodu biblioteki GUI poprzez ujednolicenie zarządzania pamięcią, obsługi zdarzeń i renderowania tekstu, a także zastosowanie ogólnych najlepszych praktyk C++.

**Obszary do Poprawy:**

1.  **Zarządzanie Pamięcią:**
    *   **Problem:** Obecnie stosowane jest mieszane podejście do zarządzania pamięcią, obejmujące surowe wskaźniki (np. dla dzieci w `GUIElement`, przycisków w `Slider`, grupy w `RadioButton`) oraz `std::shared_ptr` (dla czcionek i tekstur). Klasa `Slider` tworzy przyciski strzałek za pomocą `new`, ale nie zwalnia ich pamięci w destruktorze, co prowadzi do wycieków pamięci. `GUIManager` przechowuje surowe wskaźniki, a `main.cpp` używa `std::unique_ptr`, ale zwalnia własność (`.release()`) przy dodawaniu do menedżera, co jest niejasne i potencjalnie niebezpieczne.
    *   **Rozwiązanie:** Ujednolicenie modelu własności. Zaleca się stosowanie `std::unique_ptr` dla relacji rodzic-dziecko (gdzie rodzic jest wyłącznym właścicielem dzieci) oraz `std::shared_ptr` dla zasobów współdzielonych, takich jak czcionki i tekstury.
        *   Zmień `std::vector<GUIElement*>` m_children w `GUIElement` na `std::vector<std::unique_ptr<GUIElement>>`.
        *   Zaktualizuj metodę `addChild` w `GUIElement`, aby akceptowała `std::unique_ptr<GUIElement>` i przenosiła własność (`std::move`).
        *   Zmień przyciski `m_decreaseButton` i `m_increaseButton` w `Slider` na `std::unique_ptr<Button>` i dodaj je jako dzieci za pomocą zaktualizowanej metody `addChild`. Usunięcie `Slider` automatycznie zwolni pamięć przycisków dzięki `std::unique_ptr`.
        *   Zdecyduj, czy `GUIManager` powinien być właścicielem elementów najwyższego poziomu. Jeśli tak, zmień `std::vector<GUIElement*>` m_elements na `std::vector<std::unique_ptr<GUIElement>>` i zaktualizuj `addElement`, aby akceptował `std::unique_ptr<GUIElement>`. W `main.cpp` użyj `std::move` przy dodawaniu elementów do `GUIManager`.
        *   W `RadioButton` wskaźnik `m_group` powinien pozostać surowym wskaźnikiem, ponieważ grupa zarządza cyklem życia przycisków, a przycisk jedynie odwołuje się do swojej grupy.

2.  **Refaktoryzacja Renderowania Tekstu:**
    *   **Problem:** Logika tworzenia i aktualizacji tekstur dla etykiet/tekstu jest powielona w klasach `Checkbox`, `RadioButton` i `TextInput`. Każda klasa zarządza własną teksturą i logiką jej odświeżania, co prowadzi do redundancji i utrudnia wprowadzanie zmian.
    *   **Rozwiązanie:** Centralizacja logiki renderowania tekstu. Stwórz osobną funkcję pomocniczą (lub klasę), która przyjmuje tekst, czcionkę, kolor i renderer, a zwraca `SharedTexture`.
        *   Przenieś logikę tworzenia tekstury z `updateLabelTexture` (w `Checkbox`, `RadioButton`) i renderowania tekstu (w `TextInput`) do tej nowej funkcji/klasy.
        *   Widgety wyświetlające tekst będą przechowywać `SharedTexture` i wywoływać tę centralną funkcję, gdy tekst, czcionka lub kolor ulegną zmianie.

3.  **Spójna Obsługa Zdarzeń:**
    *   **Problem:** Logika sprawdzania, czy zdarzenie myszy miało miejsce w obrębie elementu, jest powielona w `GUIElement::handleEvent` i `GUIManager::handleEvents`. Klasa `Slider` jawnie wywołuje `handleEvent` na swoich dzieciach, co może kolidować z ogólnym mechanizmem propagacji zdarzeń w `GUIElement`. Obsługa zdarzenia "kliknięcia" różni się między widgetami (`SDL_MOUSEBUTTONDOWN` vs `SDL_MOUSEBUTTONUP`).
    *   **Rozwiązanie:** Ujednolicenie mechanizmu obsługi i propagacji zdarzeń.
        *   Upewnij się, że sprawdzanie `contains(mouseX, mouseY)` odbywa się tylko raz na odpowiednim poziomie (np. w `GUIManager` dla elementów najwyższego poziomu, a w `GUIElement::handleEvent` przed propagacją do dzieci).
        *   Usuń jawne wywołania `handleEvent` na dzieciach w klasach pochodnych (np. w `Slider`), polegając na domyślnej propagacji w `GUIElement::handleEvent`.
        *   Ujednolicenie zdarzenia "kliknięcia" - zaleca się użycie `SDL_MOUSEBUTTONUP`, ponieważ pozwala to na naciśnięcie przycisku myszy w obrębie widgetu, przeciągnięcie poza niego i zwolnienie bez wywołania akcji kliknięcia.
        *   Dostosuj logikę w `Button`, `Checkbox` i `RadioButton` do wybranej metody obsługi kliknięcia.

4.  **Czyszczenie Kodu i Najlepsze Praktyki:**
    *   **Problem:** Powielone include'y (`gui.hpp` w `checkbox.hpp` i `radio_button.hpp`).
    *   **Rozwiązanie:** Usunięcie powielonych include'ów.
    *   Dodanie brakujących include'ów, jeśli zajdzie taka potrzeba po refaktoryzacji.
    *   Przejrzenie i poprawa implementacji destruktorów, aby upewnić się, że zasoby są prawidłowo zwalniane, zwłaszcza po zmianach w zarządzaniu pamięcią.
    *   Dodanie komentarzy wyjaśniających bardziej złożone fragmenty kodu.
    *   Zastosowanie `const` tam, gdzie jest to możliwe i zasadne (const correctness).
    *   Dostosowanie przykładu użycia w `main.cpp` do nowego modelu zarządzania pamięcią (np. użycie `std::move` przy dodawaniu do menedżera/rodzica).

**Wizualizacja Hierarchii Klas:**

```mermaid
classDiagram
    GUIElement <|-- Button
    GUIElement <|-- Panel
    GUIElement <|-- Slider
    GUIElement <|-- TextInput
    GUIElement <|-- Checkbox
    GUIElement <|-- RadioButton
    RadioGroup o-- RadioButton : zarządza
    GUIManager o-- GUIElement : zarządza (opcjonalnie, zależnie od decyzji o własności)
    Slider o-- Button : zawiera (jako dzieci)