# Code Review - Analiza Kodu Źródłowego

Data: 2025-08-10

## Wprowadzenie

Ten dokument przedstawia wyniki szczegółowej analizy kodu źródłowego projektu SDL_GUI znajdującego się w katalogu `src/`. Celem analizy było zidentyfikowanie potencjalnych błędów, złych praktyk programistycznych oraz możliwości ulepszeń w kodzie.

## 1. Potencjalne Błędy

### 1.1. Nieprawidłowa obsługa zasobów SDL w `sdl_app.hpp`
*   **Problem:** W konstruktorze [`SDLApp`](src/sdl_app.hpp:12) w przypadku niepowodzenia jednej z funkcji `*_Init`, poprzednio zainicjowane biblioteki nie są prawidłowo zamykane przed rzuceniem wyjątku. W destruktorze wywoływane są funkcje `*_Quit()` w nieprawidłowej kolejności.
*   **Lokalizacja:** [`src/sdl_app.hpp:13-51`](src/sdl_app.hpp:13), [`src/sdl_app.hpp:53-59`](src/sdl_app.hpp:53)
*   **Sugestia:** Należy zapewnić, że w przypadku błędu inicjalizacji, wszystkie pomyślnie uruchomione podsystemy SDL są zamykane w odwrotnej kolejności. Warto zastosować wzorzec RAII do zarządzania cyklem życia bibliotek SDL.

### 1.2. Rysowanie tekstury w `Checkbox`
*   **Problem:** W metodzie [`Checkbox::draw`](src/checkbox.cpp:63), `SDL_RenderCopy` jest wywoływane z `nullptr` jako czwartym argumentem (prostokąt docelowy), co powoduje rysowanie na całym rendererze, a nie w granicach checkboxa.
*   **Lokalizacja:** [`src/checkbox.cpp:63`](src/checkbox.cpp:63)
*   **Sugestia:** Należy przekazać wskaźnik do prostokąta docelowego o wymiarach checkboxa, np. `SDL_Rect dstRect = {0, 0, m_width, m_height}; SDL_RenderCopy(renderer, (*style.texture).get(), nullptr, &dstRect);`.

### 1.3. Wiszące wskaźniki w `TabControl`
*   **Problem:** Klasa [`TabControl`](src/tab_control.hpp:9) przechowuje surowe wskaźniki do przycisków i paneli w `m_tabButtons` i `m_tabPanels`. Te elementy są zarządzane przez `std::unique_ptr` w klasie bazowej `GUIElement`. Jeśli element zostanie usunięty, wskaźniki w `TabControl` stają się wiszące.
*   **Lokalizacja:** [`src/tab_control.hpp:27-28`](src/tab_control.hpp:27)
*   **Sugestia:** Zamiast surowych wskaźników, należy używać `std::weak_ptr` lub identyfikatorów do bezpiecznego odwoływania się do zarządzanych elementów.

### 1.4. Nieprawidłowa obsługa UTF-8 w `TextArea` i `TextInput`
*   **Problem:** Operacje na tekście, takie jak usuwanie znaków (`Backspace`), zakładają kodowanie jednobajtowe. Wywołanie `m_text.erase(m_cursorPos - 1, 1)` uszkodzi znaki wielobajtowe w standardzie UTF-8.
*   **Lokalizacja:** [`src/text_area.cpp:147`](src/text_area.cpp:147), [`src/text_input.cpp:208`](src/text_input.cpp:208)
*   **Sugestia:** Należy zintegrować bibliotekę do obsługi UTF-8 (np. utf8.h) do prawidłowego manipulowania stringami.

## 2. Złe Praktyki Programistyczne

### 2.1. Duplikacja Kodu
*   **Problem:** Znaczna część kodu jest powielona w różnych miejscach.
    *   Logika rysowania w `AnimatedImage::draw()` i `AnimatedImage::drawDirect()`.
    *   Logika obsługi kursora i wprowadzania tekstu w `TextArea` i `TextInput`.
    *   ~~Logika rysowania tła i obramowania w prawie każdym komponencie (`Button`, `Panel`, `Checkbox`, etc.).~~
*   **Lokalizacja:** [`src/animated_image.cpp:84`](src/animated_image.cpp:84), [`src/animated_image.cpp:144`](src/animated_image.cpp:144), `src/text_area.cpp`, `src/text_input.cpp`.
*   **Sugestia:** Należy wydzielić powtarzające się fragmenty do wspólnych funkcji pomocniczych lub metod w klasach bazowych. Dla edycji tekstu warto stworzyć wspólną klasę bazową. Logikę rysowania tła i ramki można przenieść do `GUIElement::draw`.

### 2.2. "Magiczne liczby" i Hardkodowane Ścieżki
*   **Problem:** W kodzie znajduje się wiele "magicznych liczb" oraz hardkodowanych ścieżek do zasobów (np. czcionek).
*   **Lokalizacja:** [`src/gui_manager.cpp:9-12`](src/gui_manager.cpp:9), [`src/label.cpp:14`](src/label.cpp:14), [`src/text_input.cpp:85`](src/text_input.cpp:85).
*   **Sugestia:** Wartości te powinny być zdefiniowane jako stałe (`constexpr`), lub stać się częścią systemu motywów (`Theme`), aby umożliwić łatwą konfigurację i zmianę wyglądu.

### 2.3. Inicjalizacja i zamykanie bibliotek SDL
*   **Problem:** Funkcje `TTF_Init()` i `IMG_Init()` są wywoływane w konstruktorach `FontManager` i `TextureManager`. Cykl życia bibliotek SDL powinien być zarządzany centralnie.
*   **Lokalizacja:** [`src/font_manager.cpp:9`](src/font_manager.cpp:9), [`src/texture_manager.cpp:11`](src/texture_manager.cpp:11).
*   **Sugestia:** Inicjalizacja i zamykanie wszystkich systemów SDL powinno odbywać się w jednym miejscu, np. w klasie `SDLApp`.

### 2.4. Użycie `dynamic_cast` w `RadioGroup`
*   **Problem:** `RadioGroup` używa `dynamic_cast` do identyfikacji swoich dzieci jako `RadioButton`. Jest to często oznaka, że projekt hierarchii klas można ulepszyć.
*   **Lokalizacja:** [`src/radio_group.cpp:11`](src/radio_group.cpp:11), [`src/radio_group.cpp:21`](src/radio_group.cpp:21).
*   **Sugestia:** `RadioGroup` powinien utrzymywać listę `RadioButtonów` bezpośrednio, zamiast polegać na rzutowaniu.

## 3. Możliwości Ulepszeń

### 3.1. Refaktoryzacja i Uproszczenie Kodu
*   **Problem:** Niektóre funkcje i klasy są zbyt złożone.
    *   Konstruktor `Slider` zawiera dużo logiki UI.
    *   Logika przeciągania w `Panel::handleEvent`.
    *   Nieefektywne tworzenie przycisków w `ComboBox::createDropdownButtons`.
*   **Sugestia:** Warto wydzielić logikę tworzenia UI do osobnych metod. Logikę przeciągania można zamknąć w osobnej klasie `Draggable`. `ComboBox` powinien tworzyć przyciski tylko raz.

### 3.2. Wydajność
*   **Problem:** Niektóre operacje mogą być nieefektywne przy dużej liczbie elementów.
    *   `TimerManager::update` iteruje po wszystkich timerach.
    *   `TextureManager::createTextureFromText` nie cache'uje wyników.
    *   `TextArea::recalculateLines` przetwarza cały tekst przy każdej zmianie.
*   **Sugestia:**
    *   W `TimerManager` użyć kolejki priorytetowej.
    *   W `TextureManager` wprowadzić cache dla renderowanego tekstu.
    *   Zoptymalizować `TextArea` tak, aby przetwarzał tylko zmienione linie.

~~### 3.3. Wykorzystanie nowszych standardów C++~~
~~*   **Problem:** Kod używa `std::map::find` z `std::string(string_view)`, co jest nieefektywne.~~
~~*   **Lokalizacja:** [`src/font_manager.cpp:20`](src/font_manager.cpp:20), [`src/texture_manager.cpp:24`](src/texture_manager.cpp:24).~~
~~*   **Sugestia:** W C++14 i nowszych `std::map` pozwala na wyszukiwanie z typem transparentnym (`std::less<>`), co eliminuje potrzebę tworzenia tymczasowego `std::string`.~~

~~### 3.4. Użycie logowania zamiast `std::cout`~~
~~*   **Problem:** W kodzie znajduje się wiele wywołań `std::cout` do celów debugowania.~~
~~*   **Lokalizacja:** [`src/gui.cpp:270`](src/gui.cpp:270), [`src/gui_manager.cpp:99`](src/gui_manager.cpp:99).~~
~~*   **Sugestia:** Należy zastąpić je ustrukturyzowanym systemem logowania, np. `SDL_LogDebug`, `SDL_LogInfo`, `SDL_LogError`.~~