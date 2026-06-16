# Biblioteka GUI dla SDL3

## Opis

Prosta i rozszerzalna biblioteka GUI zbudowana w oparciu o SDL3 w standardzie C++23, umożliwiająca łatwe tworzenie interaktywnych elementów interfejsu użytkownika. Biblioteka została zaprojektowana z myślą o spójności, bezpieczeństwie pamięci i łatwości użycia.

## Instalacja / Zależności

### Wymagane biblioteki
- **SDL3** - okna, zdarzenia, renderowanie (GPU)
- **SDL3_image** - ładowanie obrazów (PNG, itp.) — on-demand, bez IMG_Init
- **SDL3_ttf** - renderowanie czcionek TrueType

### Wymagania kompilatora
- Kompilator obsługujący C++23 (zalecane clang++-22 z libc++)

### Instalacja (Debian/Ubuntu)
```bash
sudo apt-get install clang-22 libc++-dev libsdl3-dev libsdl3-image-dev libsdl3-ttf-dev
```

## Budowanie biblioteki

Projekt używa systemu budowania `nob.c` (technologia Go Rebuild Urself).

```bash
# Bootstrap narzędzia budującego
cc -o nob nob.c

# Budowanie przykładów (tryb debug)
./nob examples

# Budowanie i uruchamianie testów
./nob test

# Budowanie artefaktów release (biblioteki statyczne/dynamiczne w dist/)
./nob release

# Czyszczenie artefaktów
./nob clean

# Budowanie w trybie release z optymalizacjami
./nob -r examples
```

## Jak Używać

### Inicjalizacja
Aby rozpocząć, dołącz główny plik nagłówkowy `gui_manager.hpp` oraz nagłówki potrzebnych widżetów. Zalecanym sposobem jest użycie klasy pomocniczej `sdl_app.hpp`.

```cpp
#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "panel.hpp" 
// ... inne komponenty

// Inicjalizacja SDL3 i okna
SDLApp app("Moja Aplikacja", 800, 600);

// Stworzenie managera GUI
GUIManager guiManager(app.getRenderer());
```

### Główna Pętla Aplikacji
Pętla zdarzeń jest odpowiedzialnością aplikacji, co daje większą elastyczność.

```cpp
bool quit = false;
SDL_Event e;

while (!quit) {
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) {
            quit = true;
        }
        guiManager.processEvent(e);
    }

    guiManager.update();
    guiManager.cleanup(); // Bezpieczne usuwanie elementów

    SDL_SetRenderDrawColor(app.getRenderer(), 240, 240, 240, 255);
    SDL_RenderClear(app.getRenderer());
    guiManager.render();
    SDL_RenderPresent(app.getRenderer());
}
```

## Linkowanie Biblioteki

Aby użyć biblioteki, musisz zlinkować ją ze swoją aplikacją. Poniżej znajdują się przykłady dla linkowania statycznego i dynamicznego.

### Linkowanie Statyczne (`.a`)

Podczas linkowania z biblioteką statyczną (`libsdl_gui.a`) należy podać ścieżkę do plików nagłówkowych i pliku biblioteki, a także dołączyć wymagane zależności SDL3.

```bash
clang++-22 -std=c++23 twoja_aplikacja.cpp -o twoja_aplikacja -I/sciezka/do/naglowkow -L/sciezka/do/biblioteki -lsdl_gui -lSDL3 -lSDL3_image -lSDL3_ttf
```

### Linkowanie Dynamiczne (`.so`)

Podczas linkowania z biblioteką współdzieloną (`libsdl_gui.so`) upewnij się, że linker będzie w stanie ją znaleźć w czasie wykonania.

```bash
clang++-22 -std=c++23 twoja_aplikacja.cpp -o twoja_aplikacja -I/sciezka/do/naglowkow -L/sciezka/do/biblioteki -lsdl_gui -lSDL3 -lSDL3_image -lSDL3_ttf
```

Upewnij się, że plik `.so` znajduje się w katalogu znanym linkerowi dynamicznemu (np. `/usr/local/lib`) lub ustaw zmienną środowiskową `LD_LIBRARY_PATH`.

## Dostępne Komponenty

Biblioteka oferuje zestaw gotowych do użycia, w pełni konfigurowalnych komponentów (21 widgetów + 3 kompozyty + edytor).

### `Panel`
*   **Przeznaczenie:** Kontener do grupowania innych elementów, idealny jako tło dla okien lub sekcji interfejsu.
*   **Przykład użycia:**
    ```cpp
    auto panel = std::make_unique<Panel>(manager, 50, 50, 300, 200);
    panel->setBackgroundColor(ElementState::Normal, {200, 200, 200, 255});
    panel->setBorder(ElementState::Normal, {100, 100, 100, 255}, 2);
    panel->setDraggable(true);
    guiManager.addElement(std::move(panel));
    ```

### `Label`
*   **Przeznaczenie:** Statyczna etykieta tekstowa.
*   **Przykład użycia:**
    ```cpp
    auto label = std::make_unique<Label>(manager, 100, 100, "Witaj, świecie!", 24);
    guiManager.addElement(std::move(label));
    ```

### `Button`
*   **Przeznaczenie:** Standardowy przycisk reagujący na kliknięcia.
*   **Przykład użycia:**
    ```cpp
    auto button = std::make_unique<Button>(manager, 100, 150, 120, 40, "Kliknij mnie");
    button->setOnClickCallback([](GUIElement*){ 
        // Logika po kliknięciu
    });
    guiManager.addElement(std::move(button));
    ```

### `Checkbox`
*   **Przeznaczenie:** Pole wyboru (zaznaczone/odznaczone).
*   **Przykład użycia:**
    ```cpp
    auto checkbox = std::make_unique<Checkbox>(manager, 100, 200, 20, 20);
    checkbox->setOnChange([](Checkbox* cb, bool isChecked){
        // Logika po zmianie stanu
    });
    guiManager.addElement(std::move(checkbox));
    ```

### `RadioButton` i `RadioGroup`
*   **Przeznaczenie:** Grupa przycisków, z których tylko jeden może być zaznaczony.
*   **Przykład użycia:**
    ```cpp
    auto radioGroup = std::make_unique<RadioGroup>(manager, 100, 250, 200, 100);
    radioGroup->addOption("Opcja 1", true); // Domyślnie zaznaczony
    radioGroup->addOption("Opcja 2");
    guiManager.addElement(std::move(radioGroup));
    ```

### `Slider`
*   **Przeznaczenie:** Suwak do wybierania wartości z przedziału.
*   **Przykład użycia:**
    ```cpp
    auto slider = std::make_unique<Slider>(manager, 100, 380, 200, 30, 0, 100, 50, Orientation::Horizontal);
    slider->setOnChangeCallback([](GUIElement* elem){
        Slider* s = static_cast<Slider*>(elem);
        std::cout << "Wartość: " << s->getValue() << std::endl;
    });
    guiManager.addElement(std::move(slider));
    ```

### `TextInput`
*   **Przeznaczenie:** Jednoliniowe pole do wprowadzania tekstu.
*   **Przykład użycia:**
    ```cpp
    auto textInput = std::make_unique<TextInput>(manager, 100, 420, 200, 30);
    textInput->setOnEnterPressed([](TextInput* ti){
        std::cout << "Wprowadzono: " << ti->getText() << std::endl;
    });
    guiManager.addElement(std::move(textInput));
    ```

### `TextArea`
*   **Przeznaczenie:** Wieloliniowe pole tekstowe z zawijaniem wierszy.
*   **Przykład użycia:**
    ```cpp
    auto textArea = std::make_unique<TextArea>(manager, 400, 50, 300, 200, "assets/font.ttf", 16);
    textArea->setText("To jest wieloliniowy\ntekst z obsługą\nzawijania wierszy.");
    textArea->setWordWrap(true);
    guiManager.addElement(std::move(textArea));
    ```

### `ComboBox`
*   **Przeznaczenie:** Rozwijana lista opcji.
*   **Przykład użycia:**
    ```cpp
    auto comboBox = std::make_unique<ComboBox>(manager, 400, 280, 150, 30);
    comboBox->addItem("Opcja A");
    comboBox->addItem("Opcja B");
    comboBox->addItem("Opcja C");
    comboBox->on_selection_changed = [](int index, const std::string& item){
        // Logika po wybraniu opcji
    };
    guiManager.addElement(std::move(comboBox));
    ```

### `TabControl`
*   **Przeznaczenie:** Kontener z zakładkami do przełączania widoków.
*   **Przykład użycia:**
    ```cpp
    auto tabControl = std::make_unique<TabControl>(manager, 400, 330, 300, 150, 30);
    Panel* tab1 = tabControl->addTab("Zakładka 1");
    tab1->addChild(std::make_unique<Label>(manager, 10, 10, "Zawartość pierwszej zakładki.", 20));
    Panel* tab2 = tabControl->addTab("Zakładka 2");
    tab2->addChild(std::make_unique<Button>(manager, 10, 10, 100, 30, "Przycisk"));
    guiManager.addElement(std::move(tabControl));
    ```

### `ProgressBar`
*   **Przeznaczenie:** Pasek postępu.
*   **Przykład użycia:**
    ```cpp
    auto bar = std::make_unique<ProgressBar>(manager, 100, 100, 200, 30, 0.0f, 1.0f, 0.5f);
    bar->setValue(0.75f);
    bar->setShowPercentage(true);
    guiManager.addElement(std::move(bar));
    ```

### `ScrollArea`
*   **Przeznaczenie:** Przewijany kontener na zawartość większą niż obszar widoczny.
*   **Przykład użycia:**
    ```cpp
    auto scroll = std::make_unique<ScrollArea>(manager, 50, 50, 300, 200);
    auto content = std::make_unique<Panel>(manager, 0, 0, 500, 400);
    scroll->setContent(std::move(content));
    guiManager.addElement(std::move(scroll));
    ```

### `ArcContainer`
*   **Przeznaczenie:** Kontener układający elementy po łuku.
*   **Przykład użycia:**
    ```cpp
    auto arc = std::make_unique<ArcContainer>(manager, 200, 200, 150, 0.0f, 360.0f);
    arc->addChild(std::make_unique<Button>(manager, 0, 0, 60, 30, "A"));
    arc->addChild(std::make_unique<Button>(manager, 0, 0, 60, 30, "B"));
    guiManager.addElement(std::move(arc));
    ```

### `Canvas`
*   **Przeznaczenie:** Powierzchnia do rysowania.
*   **Przykład użycia:**
    ```cpp
    auto canvas = std::make_unique<Canvas>(manager, 50, 50, 400, 300);
    canvas->clear();
    guiManager.addElement(std::move(canvas));
    ```

### `AnimatedImage`
*   **Przeznaczenie:** Animacja sprite'ów.
*   **Przykład użycia:**
    ```cpp
    auto anim = std::make_unique<AnimatedImage>(manager, 100, 100, 200, 200);
    anim->setSpriteSheet("assets/sprite.png", 8, 1);
    anim->setFPS(12);
    anim->setLoop(true);
    anim->play();
    guiManager.addElement(std::move(anim));
    ```

### `ShaderPanel`
*   **Przeznaczenie:** Panel z niestandardowym shaderem GPU.
*   **Przykład użycia:**
    ```cpp
    auto shaderPanel = std::make_unique<ShaderPanel>(manager, 50, 50, 400, 300,
        vertexShaderSource, fragmentShaderSource);
    guiManager.addElement(std::move(shaderPanel));
    ```

### `StringGrid`
*   **Przeznaczenie:** Tabela danych z nagłówkami, sortowaniem i edycją.
*   **Przykład użycia:**
    ```cpp
    auto grid = std::make_unique<StringGrid>(manager, 50, 50, 400, 300, 5, 3);
    grid->setColumnHeader(0, "Nazwa");
    grid->setCellText(0, 0, "Element 1");
    guiManager.addElement(std::move(grid));
    ```

### `ListView`
*   **Przeznaczenie:** Prosta lista elementów.
*   **Przykład użycia:**
    ```cpp
    auto list = std::make_unique<ListView>(manager, 50, 50, 200, 300);
    list->addItem("Pierwszy element");
    list->addItem("Drugi element");
    guiManager.addElement(std::move(list));
    ```

### `ContextMenu`
*   **Przeznaczenie:** Menu kontekstowe (prawy przycisk myszy).
*   **Przykład użycia:**
    ```cpp
    auto menu = std::make_unique<ContextMenu>(manager);
    menu->addItem("Kopiuj", []() { std::cout << "Kopiuj" << std::endl; });
    menu->addItem("Wklej", []() { std::cout << "Wklej" << std::endl; });
    menu->showAt(mouseX, mouseY);
    ```

## Komponenty Złożone (Composite)

### `DialogBox` - Okno dialogowe
```cpp
DialogBox::createConfirm(manager, "Na pewno?", "Tak", "Nie",
    [](bool confirmed) {
        if (confirmed) { /* ... */ }
    });
```

### `MessageBox` - Komunikaty
```cpp
MessageBox::showInfo(manager, "Plik zapisany pomyślnie.");
MessageBox::showError(manager, "Błąd: Nie można otworzyć pliku.");
MessageBox::showWarning(manager, "Ostrzeżenie: Mało pamięci.");
```

### `FileDialog` - Wybór plików
```cpp
FileDialog::createOpen(manager, "Otwórz plik",
    [](const std::string& path) { /* ... */ });

FileDialog::createSave(manager, "Zapisz plik", "domyślny.txt",
    [](const std::string& path) { /* ... */ });
```

## Systemy zarządzania ekranami

### `ScreenManager` - wiele ekranów w jednym oknie (gry)
```cpp
ScreenManager screenManager(guiManager);
screenManager.addScreen("menu", std::make_unique<MenuScreen>());
screenManager.addScreen("gra", std::make_unique<GameScreen>());
screenManager.changeScreen("menu");
```

### `WindowManager` - wiele okien systemowych
```cpp
WindowManager windowManager;
Window* main = windowManager.createWindow("Główne", 800, 600);
main->getGUIManager().addElement(std::make_unique<Panel>(...));

Window* settings = windowManager.createWindow("Ustawienia", 400, 300, true);
settings->getGUIManager().addElement(std::make_unique<TextInput>(...));

while (!windowManager.shouldQuit()) {
    windowManager.processEvents();
    windowManager.updateAll();
    windowManager.renderAll();
    windowManager.cleanupAll();
}
```

## Edytor WYSIWYG

Biblioteka zawiera wizualny edytor GUI:
- **EditorWindow** - główne okno edytora z paletą widgetów
- **EditorState** - zarządzanie stanem (undo/redo, zaznaczenie, schowek)
- **PreviewWindow** - podgląd na żywo
- **LayoutImporter** - import layoutów JSON/XML
- **LayoutExporter** - eksport do JSON lub XML

## Parsery layoutów

Definicja GUI z plików JSON lub XML:

```cpp
JsonParser parser(guiManager);
auto root = parser.loadLayout("layout.json");
if (root) guiManager.addElement(std::move(root));
```

```cpp
SGMLParser parser(guiManager);
auto root = parser.loadLayout("layout.xml");
if (root) guiManager.addElement(std::move(root));
```

## Stylowanie i motywy

```cpp
Style style;
style.backgroundColor = {240, 240, 240, 255};
style.borderRadius = 8;
element->setStyle(ElementState::Normal, style);

Theme theme = Theme::createDefaultTheme();
guiManager.setTheme(theme);
```

## Indeksy dokumentacji
- Archiwum (PL): [docs/pl/archive/README.md](docs/pl/archive/README.md)
- Archive (EN): [docs/en/archive/README.md](docs/en/archive/README.md)
