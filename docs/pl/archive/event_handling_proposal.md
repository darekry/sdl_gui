# Propozycja Refaktoryzacji Systemu Obsługi Zdarzeń
[Ta strona jest dostępna po angielsku](../../en/archive/event_handling_proposal.md)
[Powrót do Archiwum](./README.md)

Data: 2025-07-06
## Wstęp

Obecny system obsługi zdarzeń w `GUIManager::handleEvents()` jest zbyt restrykcyjny. Zwraca jedynie wartość `bool` informującą, czy należy zamknąć aplikację (`SDL_QUIT`). To uniemożliwia aplikacji głównej reagowanie na zdarzenia, które nie zostały obsłużone przez żaden element GUI (np. globalne skróty klawiszowe, zdarzenia systemowe).

Niniejszy dokument proponuje refaktoryzację tego mechanizmu w celu zwiększenia jego elastyczności i oddzielenia logiki GUI od logiki aplikacji.

## 1. Problem: "Połykanie" Nieobsłużonych Zdarzeń

Metoda `GUIManager::handleEvents()` iteruje przez pętlę `SDL_PollEvent` i przekazuje zdarzenia do elementów GUI. Jeśli żadne dziecko nie obsłuży zdarzenia, jest ono po prostu odrzucane. Aplikacja główna nigdy nie ma szansy go zobaczyć, chyba że jest to `SDL_QUIT`.

## 2. Proponowane Rozwiązanie

Proponuję zmianę sposobu działania pętli zdarzeń, aby umożliwić przekazywanie nieobsłużonych zdarzeń z powrotem do pętli głównej aplikacji.

### 2.1. Zmiana Sygnatury `GUIManager::handleEvents`

Metoda `handleEvents` nie powinna już zawierać pętli `SDL_PollEvent`. Zamiast tego powinna przyjmować pojedyncze zdarzenie i zwracać `bool` informujący, czy zdarzenie zostało "skonsumowane" przez GUI.

**Obecna implementacja (pseudo-kod):**
```cpp
bool GUIManager::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) return true;
        for (auto& el : m_elements) {
            if (el->handleEvent(e)) break;
        }
    }
    return false;
}
```

**Proponowana implementacja:**
```cpp
// Zmiana nazwy dla jasności
bool GUIManager::processEvent(const SDL_Event& e) {
    for (const auto& element : m_elements) {
        if (element && element->handleEvent(e)) {
            return true; // Zdarzenie obsłużone przez GUI
        }
    }
    return false; // Zdarzenie nieobsłużone
}
```

### 2.2. Modyfikacja Głównej Pętli Aplikacji

Pętla `SDL_PollEvent` powinna zostać przeniesiona z powrotem do pętli głównej aplikacji (`main`). Pozwoli to na elastyczne zarządzanie zdarzeniami.

**Proponowana pętla główna:**
```cpp
bool quit = false;
SDL_Event e;

while (!quit) {
    while (SDL_PollEvent(&e)) {
        // 1. Najpierw pozwól GUI spróbować obsłużyć zdarzenie
        bool eventHandledByGUI = guiManager.processEvent(e);

        // 2. Jeśli GUI nie obsłużyło zdarzenia, obsłuż je w aplikacji
        if (!eventHandledByGUI) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            // Tutaj można dodać obsługę innych zdarzeń,
            // np. globalnych skrótów klawiszowych
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_F5) {
                // Odśwież dane
            }
        }
    }

    // ... reszta pętli (cleanup, render) ...
}
```

## 3. Korzyści z Proponowanych Zmian

- **Separacja odpowiedzialności:** Logika GUI zajmuje się tylko zdarzeniami GUI. Logika aplikacji zajmuje się resztą.
- **Elastyczność:** Aplikacja może reagować na dowolne zdarzenia SDL, które nie są bezpośrednio związane z klikaniem w elementy interfejsu.
- **Lepsza skalowalność:** Łatwiej będzie dodawać nowe, globalne zachowania do aplikacji bez ingerencji w kod biblioteki GUI.
- **Zgodność z oczekiwaniami:** Taki model obsługi zdarzeń jest standardem w wielu innych bibliotekach i frameworkach.