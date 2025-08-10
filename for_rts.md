# Przewodnik użycia SDL GUI do budowy gry RTS

## Krótkie wprowadzenie i cel dokumentu

Ten dokument ma na celu dostarczyć kompletne, techniczne i przystępne wprowadzenie dla zespołu programistów, którzy chcą zbudować prostą grę RTS wykorzystując bibliotekę SDL GUI zawartą w tym repozytorium. Zawiera instrukcje szybkiego startu, opis architektury, przepływu renderowania, zarządzania zasobami, animacji, timerów, obsługi zdarzeń oraz przykładowy plan projektu RTS.

Biblioteka i kluczowe pliki źródłowe znajdują się w katalogu [`src`](src/:1). Najważniejsze odniesienia użyte w tym dokumencie:
- Menedżer GUI: [`src/gui_manager.hpp`](src/gui_manager.hpp:19), [`src/gui_manager.cpp`](src/gui_manager.cpp:14)
- Baza elementów GUI: [`src/gui.hpp`](src/gui.hpp:19), [`src/gui.cpp`](src/gui.cpp:8)
- TextureManager: [`src/texture_manager.hpp`](src/texture_manager.hpp:15), [`src/texture_manager.cpp`](src/texture_manager.cpp:1)
- FontManager: [`src/font_manager.hpp`](src/font_manager.hpp:30), [`src/font_manager.cpp`](src/font_manager.cpp:7)
- AnimationManager: [`src/animation_manager.hpp`](src/animation_manager.hpp:24)
- TimerManager: [`src/timer_manager.hpp`](src/timer_manager.hpp:18), [`src/timer_manager.cpp`](src/timer_manager.cpp:1)
- Styl i motyw: [`src/style.hpp`](src/style.hpp:17), [`src/theme.hpp`](src/theme.hpp:10)
- Przykłady: katalog [`examples/`](examples/:1) (zawiera m.in. [`examples/example_button.cpp`](examples/example_button.cpp:1), [`examples/example_animation.cpp`](examples/example_animation.cpp:1))

## Szybki start: minimalny krok-po-kroku

Wymagania środowiskowe:
- SDL2 (biblioteka developerska)
- SDL_image
- SDL_ttf

Najszybszy sposób uruchomienia przykładów:
1. Zainstaluj zależności systemowe (np. na Debian/Ubuntu):
   sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
2. Przejdź do katalogu projektu i zbuduj:
   make
   (Szczegóły kompilacji i targetów zawiera [`Makefile`](Makefile:1))
3. Uruchom przykład, np.:
   ./output/example_button

Pliki przykładowe można przeglądać w [`examples/`](examples/:1). Przydatne: [`examples/example_button.cpp`](examples/example_button.cpp:1), [`examples/example_animation.cpp`](examples/example_animation.cpp:1), [`examples/example_panel.cpp`](examples/example_panel.cpp:1).

Uwaga: jeśli CI lub środowisko jest headless, zobacz strategię testów w [`.kilocode/rules/memory-bank/testing_strategy.md`](.kilocode/rules/memory-bank/testing_strategy.md:1) oraz użyj Xvfb do uruchamiania renderingu w trybie headless.

## Architektura biblioteki — kluczowe komponenty i pliki

1) GUIManager
- Centralny punkt aplikacji — inicjalizuje SDL renderer, menedżery zasobów i zarządza top-level elementami GUI.
- Główne pliki: [`src/gui_manager.hpp`](src/gui_manager.hpp:19), [`src/gui_manager.cpp`](src/gui_manager.cpp:14)
- Metody: inicjalizacja, `render()`, `pollEvents()` i `addElement()` — rejestracja widgetów w menedżerze.

2) GUIElement
- Abstrakcyjna baza dla widgetów: pozycja, rozmiar, widoczność, hierarchia rodzic-dziecko.
- Pliki: [`src/gui.hpp`](src/gui.hpp:19), [`src/gui.cpp`](src/gui.cpp:8)
- Kluczowe koncepcje: markDirty, markForDeletion, wantsDirectRender(), draw()/drawDirect(), renderToCache(), `m_cachedTexture`.

3) TextureManager
- Ładowanie obrazów i tworzenie `SharedTexture` z cache.
- Pliki: [`src/texture_manager.hpp`](src/texture_manager.hpp:15), [`src/texture_manager.cpp`](src/texture_manager.cpp:1)
- Zapewnia funkcje: loadTexture(path), getTexture(key), createDefaultTexture().

4) FontManager
- Cache czcionek i pomocnicze API do mierzenia rozmiaru tekstu (`getTextSize`).
- Pliki: [`src/font_manager.hpp`](src/font_manager.hpp:30), [`src/font_manager.cpp`](src/font_manager.cpp:7)

5) AnimationManager
- System animacji operujący na polach typu int/float/variant oraz easing.
- Plik: [`src/animation_manager.hpp`](src/animation_manager.hpp:24)

6) TimerManager
- Harmonogram zdarzeń czasowych (single-shot, interval) powiązanych z elementami GUI.
- Pliki: [`src/timer_manager.hpp`](src/timer_manager.hpp:18), [`src/timer_manager.cpp`](src/timer_manager.cpp:1)

7) Style i Theme
- Centralizacja stylów i motywów: [`src/style.hpp`](src/style.hpp:17), [`src/theme.hpp`](src/theme.hpp:10)

8) Przykładowe widgety i wykorzystanie
- Przykład tworzenia widgetu: [`examples/example_button.cpp`](examples/example_button.cpp:1)
- Animacje: [`examples/example_animation.cpp`](examples/example_animation.cpp:1)
- Więcej przykładów: [`examples/`](examples/:1)

## Przepływ renderowania i lifecycle elementów

Zarys:
- `GUIManager::render()` iteruje po top-level elementach i wywołuje `GUIElement::render()` — zobacz [`src/gui_manager.cpp`](src/gui_manager.cpp:51) i [`src/gui.cpp`](src/gui.cpp:135).
- `GUIElement::render()` sprawdza `wantsDirectRender()`:
  - Jeśli true: ustawia clipping i wywołuje `drawDirect(renderer)`, renderuje dzieci bez cache.
  - Jeśli false: jeśli `m_isDirty` -> `renderToCache()` (ustawia render target na `m_cachedTexture`), wywołuje `draw(renderer)` i resetuje target. Implementacja znajduje się w [`src/gui.cpp`](src/gui.cpp:135).
- Po wyrenderowaniu `m_cachedTexture` jest kopiowany na główny renderer (SDL_RenderCopy) a następnie element renderuje dzieci z odpowiednim clippingiem.
- Cleanup: `GUIElement::cleanup()` i `GUIManager::cleanup()` usuwają elementy oznaczone jako do usunięcia (markForDeletion) i zwalniają timery/animacje — zobacz [`src/gui.cpp`](src/gui.cpp:267) oraz [`src/gui_manager.cpp`](src/gui_manager.cpp:64).

Wnioski praktyczne:
- Używaj cache'owania (`m_cachedTexture`) dla skomplikowanych, rzadko zmieniających się widgetów (mapy, HUD), aby zredukować koszt draw() każdego frame.
- Użyj `drawDirect()`/wantsDirectRender() dla dynamicznych elementów interaktywnych, które wymagają rysowania dzieci w kontekście frame (np. kursory, efekty cząsteczkowe).
- Zadbaj o rekreację cache przy zmianie rozmiaru widgetu — implementacja w [`src/gui.cpp`](src/gui.cpp:220).

## Zarządzanie zasobami: TextureManager i FontManager

TextureManager:
- API: ładowanie przez `loadTexture(path)` i odpytywanie `getTexture(key)` — definicje w [`src/texture_manager.hpp`](src/texture_manager.hpp:15).
- Zwraca `SharedTexture` (std::shared_ptr<SDL_Texture>) i przechowuje mapę cache.
- Najlepsze praktyki:
  - Używaj spójnych kluczy/ścieżek zasobów (np. "units/soldier.png").
  - Twórz atlasy tekstur tam, gdzie to możliwe, aby ograniczyć przełączanie tekstur renderer'a.
  - Zarejestruj domyślną teksturę do gui w `GUIManager` — zobacz [`src/gui_manager.cpp`](src/gui_manager.cpp:14).
- Debugowanie problemów z assetami:
  - Sprawdź czy plik istnieje i ścieżka jest poprawna.
  - Zaloguj wynik `SDL_LogError` z TextureManager (menedżer powinien logować błędy).
  - Jeśli tekstura nie powstaje, sprawdź inicjalizację `SDL_image`.
  - Ścieżka domyślnej czcionki/assetów: np. `assets/fonts/font.ttf` — TODO: weryfikacja dokładnej polityki publikacji zasobów (zaznaczono dalej).

FontManager:
- API: `loadFont(path, size)` i `getTextSize(font, text)` — definicje w [`src/font_manager.hpp`](src/font_manager.hpp:30).
- Cache: `SharedFont` w mapie, synchronizacja z `SDL_ttf`.
- Debug: błędy inicjalizacji `TTF_OpenFont` są typowe — sprawdź logi i czy plik czcionki jest dostępny.

## System animacji

- AnimationManager obsługuje animacje prostych właściwości (int/float/variant) oraz easing. Główne API w [`src/animation_manager.hpp`](src/animation_manager.hpp:24).
- Właściwości wspierane:
  - Liczby całkowite i zmiennoprzecinkowe
  - std::variant do reprezentacji wartości heterogenicznych
  - Easing: podstawowe funkcje easing (patrz [`src/easing.hpp`](src/easing.hpp:1))
- Lifecycle animacji:
  - Tworzenie animacji: zarejestruj cel, wartość początkową, końcową, czas trwania, easing oraz optional callback on_complete.
  - Aktualizacja: `AnimationManager::update(dt)` jest wywoływany codziennie (Frame tick) przez `GUIManager`.
  - Usuwanie: animacje kończą się i są usuwane automatycznie po wykonczeniu lub mogą być przerwane.
- Praktyczny przykład znajdziesz w [`examples/example_animation.cpp`](examples/example_animation.cpp:1).

Wskazówki projektowe:
- Używaj animacji dla przejść stanów jednostek (idle -> walk -> attack) i dla płynnych ruchów kamery.
- Synchronizuj klatki animacji z teksturami przez `TextureManager` (np. atlas animacji) i aktualizację indeksu klatki w callbacku animacji.

## TimerManager: użycie timerów w logice gry

- TimerManager udostępnia timery typu single-shot i interval (cykliczne). Zobacz [`src/timer_manager.hpp`](src/timer_manager.hpp:18).
- API typowo: `startTimer(ownerId, intervalMs, singleShot, callback)` i `stopTimer(id)`.
- Powiązanie z GUIElementami:
  - Elementy GUI mogą uruchamiać timery przez `startTimer`, a TimerManager powiadamia callback powiązany z elementem.
  - Timery używane do: ruchu jednostek (ticks), cooldownów ataku, AI ticków, animacji sterowanych czasem.

Przykład użycia: patrolująca jednostka może uruchamiać timer o interwale 50ms by aktualizować pozycję i uruchamiać odpowiednie animacje.

## Obsługa zdarzeń i klikalność

- Bazowy typ `GUIElement` w [`src/gui.hpp`](src/gui.hpp:19) udostępnia mechanizmy detekcji zdarzeń i domyślne hooki.
- Integracja z pętlą zdarzeń SDL: `GUIManager` integruje SDL event loop i przekazuje zdarzenia do elementów (zobacz [`src/gui_manager.hpp`](src/gui_manager.hpp:19)).
- Przykłady:
  - Przycisk: [`examples/example_button.cpp`](examples/example_button.cpp:1) pokazuje, jak zarejestrować callback kliknięcia.
  - Checkbox, slider i inne widgety znajdują się w [`examples/`](examples/:1).

Implementacja klikalnych jednostek:
- Dziedzicz po `GUIElement` i nadpisz metody obsługi zdarzeń (np. onMouseDown/onMouseUp/onMouseMove) — sekcja kodu poniżej zawiera przykład.
- Drag selection (selektowanie przez przeciągnięcie): implementuj prosty rectangle selection w globalnych współrzędnych świata, rysuj overlay w `drawDirect()` i podczas mouse up przelicz które jednostki są w prostokącie.
- Mapowanie współrzędnych: przemapuj współrzędne ekranu (pixel window) na world-coordinates używając transformacji kamery (offset + scale) — zobacz przykładowy pseudokod.

Fragmenty referencyjne: [`examples/example_button.cpp`](examples/example_button.cpp:1) i [`docs/creating_new_widget.md`](docs/creating_new_widget.md:1) zawierają praktyczne wskazówki.

## Przykładowy projekt RTS — architektura i pseudokod

Proponowana struktura klas:
- UnitWidget : dziedziczy po [`src/gui.hpp`](src/gui.hpp:19)
- BuildingWidget : dziedziczy po [`src/gui.hpp`](src/gui.hpp:19)
- HUD : panel z elementami GUI (pasek zasobów, przyciski) — extends [`src/panel.hpp`](src/panel.hpp:1)
- Minimap : lekki panel rysujący mapę świata bezpośrednio (może używać `drawDirect()`)

Rejestracja elementów:
- Dodaj elementy do menedżera przez `GUIManager::addElement(element)` — implementacja w [`src/gui_manager.hpp`](src/gui_manager.hpp:19) / [`src/gui_manager.cpp`](src/gui_manager.cpp:14).

Inicjalizacja zasobów (przykład):
- Load tekstury jednostek:
  auto soldierTex = guiManager.textureManager.loadTexture("assets/units/soldier.png");
- Load atlas animacji i zainicjuj animacje poprzez AnimationManager.
- Przykładowe pliki referencyjne: [`src/texture_manager.hpp`](src/texture_manager.hpp:15), [`examples/example_animation.cpp`](examples/example_animation.cpp:1).

Logika klikalności i issue commands:
- Wybieranie: kliknięcie `onMouseDown` pojedynczej jednostki ustawia selected = true.
- Grupowanie: shift+click dodaje do selekcji, drag-selection wybiera wiele jednostek na prostokącie.
- Issue command (move/attack): klik na pusty teren -> oblicz punkt docelowy w world-coordinates i ustaw dla każdej wybranej jednostki cel ruchu.
- Mapowanie click -> world:
  worldX = (screenX / zoom) + cameraOffsetX
  worldY = (screenY / zoom) + cameraOffsetY

Ruch jednostek — uproszczona symulacja:
- Użyj `TimerManager` do ticków ruchu (np. 30-60ms) lub animacji movement z `AnimationManager`.
- Prosty algorytm:
  - compute direction = normalize(target - pos)
  - pos += direction * (speed * dt)
  - jeśli dist(pos, target) < threshold => stop, change state to idle
- Synchronizuj stan z `AnimationManager` (walk->idle).

Synchronizacja animacji z atlasem:
- Trzymaj indeks klatki animacji w UnitWidget i zmieniaj go w callbacku animacji albo w ticku TimerManager.
- TextureManager może przechowywać atlas oraz zwracać subrecty dla każdej klatki.

Timery używane w AI / pathfinding / attack cooldown:
- AI tick: intervalowy timer (np. 200ms) do aktualizacji decyzji.
- Pathfinding tick: timer inicjowany gdy jednostka ma cel, aktualizuje pozycję co n ms.
- Attack cooldown: single-shot timer z callbackiem resetującym możliwość ataku.

Minimalny, kompilowalny pseudokod C++ (schematyczny)

#include "src/gui.hpp"
#include "src/gui_manager.hpp"

class UnitWidget : public GUIElement {
public:
  UnitWidget(GUIManager* mgr) : GUIElement(), manager(mgr) {
    tex = manager->textureManager.loadTexture("assets/units/soldier.png");
  }
  void draw(SDL_Renderer* r) override {
    // rysuj aktualną ramkę z tex
    SDL_Rect dst{int(pos.x), int(pos.y), w, h};
    // zakładamy, że tex zawiera atlas i metoda getFrameRect zwraca SDL_Rect
    auto frame = manager->textureManager.getFrameRect("soldier_walk", frameIndex);
    SDL_RenderCopy(r, tex.get(), &frame, &dst);
  }
  void onMouseDown(int sx, int sy) override {
    selected = true;
  }
  void moveTo(float x, float y) {
    target = {x,y};
    // uruchom timer ruchu
    manager->timerManager.startTimer(this, 50, false, [this](int){ tickMove(); });
    // uruchom animację walk
    manager->animationManager.animate(&frameIndex, frameIndex, 10, 300, easing::linear);
  }
private:
  void tickMove() {
    // prosty krok ruchu
  }
  GUIManager* manager;
  SharedTexture tex;
  int frameIndex = 0;
  bool selected = false;
  Vec2 pos, target;
};

// Rejestracja:
auto unit = std::make_shared<UnitWidget>(&guiManager);
guiManager.addElement(unit);

## Praktyczne fragmenty kodu i wzorce użycia

1) Tworzenie klasy jednostki dziedziczącej po GUIElement  
Zobacz przykład powyżej i bazę [`src/gui.hpp`](src/gui.hpp:19).

2) Zarejestrowanie i użycie TextureManager i FontManager  
auto& texMgr = guiManager.textureManager; // referencja  
auto tex = texMgr.loadTexture("assets/units/soldier.png");  
auto& fontMgr = guiManager.fontManager;  
auto font = fontMgr.loadFont("assets/fonts/font.ttf", 14);

3) Uruchomienie animacji przez AnimationManager  
manager->animationManager.animate(&pos.x, pos.x, dest.x, durationMs, easing::linear, on_complete);

4) Ustawienie i obsługa timera przez TimerManager  
manager->timerManager.startTimer(this, 100, false, [this](int id){
  // tick logic
});

5) Obsługa kliknięcia / drag selection i mapowanie koordynatów
// przy mouse down
int sx, sy; // screen coords
float worldX = (sx / camera.zoom) + camera.offsetX;
float worldY = (sy / camera.zoom) + camera.offsetY;
// potem sprawdz czy punkt należy do elementu: element->contains(worldX, worldY)

## Optymalizacje i ograniczenia

- Cache'owanie (`m_cachedTexture`) i kiedy użyć `drawDirect`:
  - Użyj cache dla statycznych lub rzadko zmieniających się widgetów (mapa, HUD). Implementacja w [`src/gui.cpp`](src/gui.cpp:135).
  - `drawDirect` gdy element potrzebuje natychmiastowego rysowania bez kopii do textury.
- Zalecenia dotyczące tekstur:
  - Rozmiary tekstur: trzymać je w mocno ograniczonych rozmiarach (potrzebny mipmaping/wielkość tekstury zależy od platformy).
  - Używaj atlasów, aby minimalizować przełączanie tekstury i draw calls.
  - Batchuj rysowania kiedy to możliwe.
- Minimalizacja zmian targetu renderera: tworzenie/rekreowanie tekstury celów jest kosztowne — unikaj tego w gorącej ścieżce.
- Testy i CI (headless): sprawdź [`.kilocode/rules/memory-bank/testing_strategy.md`](.kilocode/rules/memory-bank/testing_strategy.md:1) — użyj Xvfb w CI.

## Debugowanie i typowe problemy

- Nieudane tworzenie tekstury:
  - Sprawdź logi SDL_LogError; sprawdź zwracane nullptr z `SDL_CreateTexture`.
  - Upewnij się, że renderer został poprawnie utworzony przed wywołaniem TextureManager.
- Brak fontów:
  - Sprawdź ścieżkę do `assets/fonts/font.ttf`. Jeśli występuje błąd, FontManager powinien logować problem.
- Błędy inicjalizacji SDL_image/SDL_ttf:
  - Upewnij się, że `IMG_Init(...)` i `TTF_Init()` zwróciły sukces, loguj odpowiednio.
- Problemy z cache'owaniem:
  - Jeśli widget nie odświeża się po zmianie danych, sprawdź czy wywoływane jest `markDirty()` i czy wielkość `m_cachedTexture` odpowiada nowemu rozmiarowi (patrz [`src/gui.cpp`](src/gui.cpp:220)).

## Checklisty i best-practices dla zespołu RTS

- Nazewnictwo zasobów:
  - assets/units/<unit_name>_walk_0.png, ... _n.png lub atlas: assets/units/<unit_name>.atlas.png
- Separacja logiki gry od widoku:
  - Trzymaj logikę AI/pathfinding w niezależnych klasach/service'ach; GUIElement powinien jedynie wizualizować stan.
- Lifecycle: zawsze oznacz element do usunięcia przez `markForDeletion()` zamiast natychmiastowego delete; GUIManager wykona cleanup (`src/gui.cpp` i `src/gui_manager.cpp`).
- Testy jednostkowe: dodaj testy dla managers (TextureManager, FontManager, AnimationManager, TimerManager).

## Linki do plików referencyjnych w repozytorium
- [`Makefile`](Makefile:1)
- [`src/gui_manager.hpp`](src/gui_manager.hpp:19)
- [`src/gui_manager.cpp`](src/gui_manager.cpp:14)
- [`src/gui.hpp`](src/gui.hpp:19)
- [`src/gui.cpp`](src/gui.cpp:135)
- [`src/texture_manager.hpp`](src/texture_manager.hpp:15)
- [`src/texture_manager.cpp`](src/texture_manager.cpp:1)
- [`src/font_manager.hpp`](src/font_manager.hpp:30)
- [`src/font_manager.cpp`](src/font_manager.cpp:7)
- [`src/animation_manager.hpp`](src/animation_manager.hpp:24)
- [`src/timer_manager.hpp`](src/timer_manager.hpp:18)
- [`src/style.hpp`](src/style.hpp:17)
- [`src/theme.hpp`](src/theme.hpp:10)
- Przykłady: [`examples/example_button.cpp`](examples/example_button.cpp:1), [`examples/example_animation.cpp`](examples/example_animation.cpp:1), [`examples/example_panel.cpp`](examples/example_panel.cpp:1)
- Memory bank test strategy: [`.kilocode/rules/memory-bank/testing_strategy.md`](.kilocode/rules/memory-bank/testing_strategy.md:1)

## Gotowy przykład — plan działania (kroki do natychmiastowego startu)

Szybki plan (pierwszy sprint, 2 tygodnie, zespół 3-5 osób):
1. Loader zasobów
   - Task: Zaimplementować loader i strukturę katalogu assets; zapewnić fallbacky w TextureManager/FontManager.
   - Referencje: [`src/texture_manager.hpp`](src/texture_manager.hpp:15), [`src/font_manager.hpp`](src/font_manager.hpp:30)
2. System jednostek (pozycja, render, selekcja)
   - Task: UnitWidget, podstawowa logika wyboru i renderu.
   - Referencje: [`src/gui.hpp`](src/gui.hpp:19), [`examples/example_button.cpp`](examples/example_button.cpp:1)
3. Selektowanie i grupowanie
   - Task: Drag selection + shift+click.
4. Ruch jednostek i prosta symulacja (timer/animation)
   - Task: TimerManager tick do aktualizacji pozycji, AnimationManager do zmian stanu.
   - Referencje: [`src/timer_manager.hpp`](src/timer_manager.hpp:18), [`src/animation_manager.hpp`](src/animation_manager.hpp:24)
5. HUD i podstawowy interfejs
   - Task: Panel zasobów, przyciski produkcji.
6. Minimap i kamera
   - Task: Minimap z bezpośrednim rysowaniem i zoom/pan kamery.

Minimalny backlog zadań (konkretne story):
- [ ] Przygotować strukturę assets i przykładowe tekstury (soldier, building).
- [ ] Dodać UnitWidget example w `examples/` oraz test integracyjny podstawowego selekcjonowania.
- [ ] Implementować TimerManager-driven movement dla jednostek.
- [ ] Podstawowy HUD z kilkoma przyciskami i licznikiem zasobów.

## TODOs i kwestie do weryfikacji

- TODO: weryfikacja dokładnej polityki publikacji zasobów i przykładowej ścieżki `assets/fonts/font.ttf`.
- TODO: sprawdzić dokładne linie implementacji cache w [`src/gui.cpp`](src/gui.cpp:135) i ewentualnie zaktualizować ten dokument, jeśli implementacja się zmieni.

------------------------------------------------------------

Dokument przygotowany jako kompletny przewodnik startowy dla zespołu RTS używającego SDL GUI. Wszystkie referencje do plików źródłowych i przykładów umieszczono jako odnośniki do repozytorium.

Koniec dokumentu.