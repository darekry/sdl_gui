# Użycie SDL GUI w Grach RTS

Ten dokument przedstawia przegląd wykorzystania biblioteki SDL GUI do tworzenia interfejsów użytkownika w grach strategicznych czasu rzeczywistego (RTS). Skupia się na publicznym API najważniejszych komponentów.

## Kluczowe Komponenty dla Interfejsów RTS

### AnimationManager
`AnimationManager` służy do tworzenia płynnych efektów wizualnych, takich jak ruchy jednostek, przejścia w interfejsie czy zmiany pasków zdrowia.

```cpp
// Animacja właściwości od wartości początkowej do końcowej w danym czasie
animationManager.createAnimation(
    &property,
    startValue,
    endValue,
    duration_ms,
    Easing::Type::Linear,
    []() { /* callback po zakończeniu */ }
);
```

### AnimatedImage
Idealny do animacji jednostek. Wyświetla animacje z arkusza sprite'ów.

```cpp
auto unitSprite = std::make_unique<AnimatedImage>(guiManager, x, y, w, h);
unitSprite->setSpriteSheet("sciezka/do/sprite.png", totalFrames, rows);
unitSprite->setFPS(12.0f);
unitSprite->setLoop(true);
unitSprite->play();
```

### MouseCursor
Umożliwia tworzenie niestandardowych, kontekstowych kursorów, które są kluczowe w grach RTS.

```cpp
// Włącz i skonfiguruj niestandardowy kursor
guiManager.setCustomCursorEnabled(true);
MouseCursor* cursor = guiManager.getMouseCursor();

cursor->setCursorTexture(CursorState::Normal, "assets/cursor_normal.png");
cursor->setCursorTexture(CursorState::Hover, "assets/cursor_hover.png");
cursor->setAnimatedCursor(CursorState::Busy, "assets/cursor_busy.png", 8, 1, 12.0f);
```

### Panel
Podstawowy kontener do budowania złożonych układów interfejsu, takich jak główny HUD.

```cpp
auto hudPanel = std::make_unique<Panel>(guiManager, 0, 500, 800, 100);
hudPanel->setBackgroundColor(ElementState::Normal, {30, 30, 30, 200});
```

### Button
Używany do przycisków akcji, szkolenia jednostek i innych interaktywnych elementów.

```cpp
auto trainButton = std::make_unique<Button>(guiManager, 10, 10, 100, 30, "Szkol jednostkę");
trainButton->setOnClickCallback([](GUIElement*) {
    // Logika szkolenia jednostki
});
```

### ContextMenu
Dostarcza menu kontekstowe dla komend jednostek (np. ruch, atak, budowa).

```cpp
auto unitMenu = std::make_unique<ContextMenu>(guiManager);
unitMenu->addItem("Atak", []() { /* ... */ });
unitMenu->addItem("Ruch", []() { /* ... */ });
unitMenu->showAt(mouseX, mouseY);
```

### TabControl
Przydatny do organizowania złożonych paneli, takich jak menu budowy czy drzewka technologiczne.

```cpp
auto buildMenu = std::make_unique<TabControl>(guiManager, 100, 100, 400, 300);
Panel* buildingsTab = buildMenu->addTab("Budynki");
Panel* unitsTab = buildMenu->addTab("Jednostki");
```

## Budowa Podstawowego HUD-a RTS

Typowy HUD w grze RTS można zbudować, łącząc elementy `Panel`, `Button` i `Label`.

1.  **Główny Panel HUD**: Stwórz główny `Panel` zadokowany na dole ekranu, który będzie głównym kontenerem.
2.  **Minimapa**: Użyj `Panel` lub niestandardowego widgetu do rysowania minimapy, zazwyczaj umieszczonej w rogu.
3.  **Wyświetlanie Zasobów**: Użyj elementów `Label` wewnątrz `Panel` do pokazywania zasobów, takich jak złoto, drewno czy zaopatrzenie.
4.  **Panel Komend**: Zgrupuj elementy `Button` w `Panel`, aby wyświetlić dostępne akcje dla zaznaczonej jednostki lub budynku.

```cpp
// 1. Stwórz główny panel HUD
auto mainHud = std::make_unique<Panel>(guiManager, 0, 500, 800, 100);

// 2. Stwórz panel na przyciski komend
auto commandGrid = std::make_unique<Panel>(guiManager, 600, 10, 190, 80);

// 3. Dodaj przyciski komend do siatki
auto attackButton = std::make_unique<Button>(guiManager, 0, 0, 40, 40, "A");
commandGrid->addChild(std::move(attackButton));

// 4. Dodaj siatkę komend do głównego HUD-a
mainHud->addChild(std::move(commandGrid));

guiManager.addElement(std::move(mainHud));
```

Taka struktura zapewnia czysty i zorganizowany sposób na budowę elastycznego i responsywnego interfejsu użytkownika w grze RTS.