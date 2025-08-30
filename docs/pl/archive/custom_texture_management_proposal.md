# Propozycja Zarządzania Teksturami Użytkownika
[Ta strona jest dostępna po angielsku](../../en/archive/custom_texture_management_proposal.md)
[Powrót do Archiwum](./README.md)

Data: 2025-07-06
## Wstęp

Biblioteka powinna umożliwiać użytkownikom tworzenie własnych, proceduralnie generowanych tekstur (np. ikony, gradienty, niestandardowe tła) i dodawanie ich do `TextureManager`. Kluczowe jest zapewnienie, że menedżer będzie poprawnie zarządzał cyklem życia tych tekstur, zwalniając pamięć, gdy nie będą już używane.

## Problem do rozwiązania

1.  **Dodawanie tekstur:** Jak umożliwić użytkownikowi dodanie własnej, istniejącej `SDL_Texture*` do `TextureManager` pod unikalnym kluczem (ID)?
2.  **Zarządzanie cyklem życia:** Jak zapewnić, że tekstura zostanie automatycznie zniszczona (za pomocą `SDL_DestroyTexture`), gdy ostatni element GUI przestanie jej używać?
3.  **Spójność API:** Jak zaprojektować API, aby było intuicyjne i spójne z istniejącymi metodami, takimi jak `loadTexture`?

Poniżej przedstawiono trzy możliwe podejścia do rozwiązania tego problemu.

---

## Rozwiązanie 1: Jawne Dodawanie `SharedTexture`

W tym podejściu użytkownik jest odpowiedzialny za opakowanie surowego wskaźnika `SDL_Texture*` w `std::shared_ptr` z odpowiednim deleterem (`SDLTextureDeleter`). Następnie przekazuje ten inteligentny wskaźnik do menedżera.

### Proponowane zmiany w API

```cpp
// W klasie TextureManager
class TextureManager {
public:
    // ... istniejące metody ...

    // Nowa metoda do dodawania istniejącego shared_ptr
    void addTexture(const std::string& key, SharedTexture texture);

    // Metoda do pobierania tekstury po kluczu
    SharedTexture getTexture(const std::string& key);
};
```

### Przykład użycia

```cpp
// Użytkownik tworzy teksturę
SDL_Texture* rawTexture = createMyCustomTexture(renderer); // funkcja użytkownika

// Użytkownik tworzy shared_ptr z deleterem
SharedTexture customTexture(rawTexture, SDLTextureDeleter());

// Użytkownik dodaje teksturę do menedżera
textureManager.addTexture("close_icon", customTexture);

// ...

// Element GUI pobiera i używa tekstury
auto buttonTexture = textureManager.getTexture("close_icon");
closeButton->setTexture(buttonTexture);
```

### Analiza

-   **Zalety:**
    -   **Jawność i kontrola:** Użytkownik ma pełną kontrolę nad tworzeniem `shared_ptr` i jest świadomy, że przekazuje własność.
    -   **Prostota implementacji:** Menedżer po prostu przechowuje przekazany `shared_ptr` w swojej mapie.
-   **Wady:**
    -   **Więcej pracy po stronie użytkownika:** Użytkownik musi pamiętać o poprawnym utworzeniu `shared_ptr` z deleterem. Jest to podatne na błędy.
    -   **Mniej przyjazne API:** Wymaga od użytkownika znajomości szczegółów implementacyjnych (`SharedTexture`, `SDLTextureDeleter`).
---
## Rozwiązanie 2: Przejęcie Własności nad Surowym Wskaźnikiem (Rekomendowane)

W tym podejściu użytkownik przekazuje do menedżera surowy wskaźnik `SDL_Texture*`, a menedżer sam bierze na siebie odpowiedzialność za opakowanie go w `std::shared_ptr` i zarządzanie jego cyklem życia.

### Proponowane zmiany w API

```cpp
// W klasie TextureManager
class TextureManager {
public:
    // ... istniejące metody ...

    // Nowa metoda, która przejmuje własność nad surowym wskaźnikiem
    // Zwraca SharedTexture, aby użytkownik mógł go od razu użyć.
    SharedTexture addTexture(const std::string& key, SDL_Texture* texture);

    // Metoda do pobierania tekstury po kluczu (bez zmian)
    SharedTexture getTexture(const std::string& key);
};
```

### Implementacja w `TextureManager`

```cpp
SharedTexture TextureManager::addTexture(const std::string& key, SDL_Texture* texture) {
    if (m_textures.count(key)) {
        // Klucz już istnieje, zwróć istniejącą teksturę, aby uniknąć wycieku pamięci
        // (nowa tekstura nie zostanie dodana)
        return m_textures[key];
    }
    if (!texture) {
        return nullptr;
    }
    SharedTexture shared(texture, SDLTextureDeleter());
    m_textures[key] = shared;
    return shared;
}
```

### Przykład użycia

```cpp
// Użytkownik tworzy teksturę
SDL_Texture* rawTexture = createMyCustomTexture(renderer);

// Użytkownik dodaje teksturę do menedżera, przekazując własność.
// Może od razu użyć zwróconego shared_ptr.
SharedTexture buttonTexture = textureManager.addTexture("close_icon", rawTexture);
closeButton->setTexture(buttonTexture);

// Użytkownik nie musi się już martwić o zwalnianie rawTexture.
```

### Analiza

-   **Zalety:**
    -   **Proste i czyste API:** Użytkownik nie musi znać `std::shared_ptr` ani deleterów. API jest bardzo intuicyjne.
    -   **Jasny kontrakt:** Przekazanie surowego wskaźnika do metody `addTexture` jasno sygnalizuje, że menedżer przejmuje własność.
    -   **Mniejsze ryzyko błędu:** Zmniejsza prawdopodobieństwo, że użytkownik zapomni o deleterze lub nieprawidłowo utworzy `shared_ptr`.
-   **Wady:**
    -   **Ukryta magia:** Użytkownik musi ufać, że menedżer poprawnie zarządza pamięcią. Jest to jednak standardowa praktyka w dobrze zaprojektowanych bibliotekach.
---
## Rozwiązanie 3: Użycie `std::weak_ptr` do Automatycznego Czyszczenia Cache'u

To podejście jest rozszerzeniem Rozwiązania 2. `TextureManager` przechowuje `std::weak_ptr` do tekstur zamiast `std::shared_ptr`. Pozwala to menedżerowi wykryć, kiedy tekstura nie jest już używana przez żaden element (licznik referencji `shared_ptr` spada do zera) i usunąć ją ze swojego wewnętrznego cache'u.

### Proponowane zmiany w API

API dla użytkownika pozostaje **identyczne jak w Rozwiązaniu 2**. Zmienia się tylko wewnętrzna implementacja menedżera.

### Implementacja w `TextureManager`

```cpp
class TextureManager {
public:
    // ... API bez zmian ...

private:
    SDL_Renderer* m_renderer;
    // Mapa przechowuje teraz weak_ptr
    std::map<std::string, std::weak_ptr<SDL_Texture>> m_textures;
};

// Metoda getTexture (lub loadTexture) musi zostać zmodyfikowana
SharedTexture TextureManager::getTexture(const std::string& key) {
    if (m_textures.count(key)) {
        // Spróbuj "zablokować" weak_ptr, aby uzyskać shared_ptr
        SharedTexture texture = m_textures[key].lock();
        if (texture) {
            // Udało się, tekstura wciąż istnieje w pamięci
            return texture;
        } else {
            // Nie udało się, tekstura została zniszczona. Usuń nieaktualny wpis.
            m_textures.erase(key);
        }
    }
    // Jeśli klucza nie ma lub był nieaktualny, załaduj teksturę (jeśli to loadTexture)
    // lub zwróć nullptr.
    return nullptr; // lub załaduj z pliku, jeśli to loadTexture
}

SharedTexture TextureManager::addTexture(const std::string& key, SDL_Texture* texture) {
    // ... (logika sprawdzania istnienia klucza) ...
    SharedTexture shared(texture, SDLTextureDeleter());
    m_textures[key] = shared; // Zapisz weak_ptr do mapy
    return shared;
}
```

### Analiza

-   **Zalety:**
    -   **Automatyczne zarządzanie cache'em:** Menedżer nie przechowuje niepotrzebnie w pamięci tekstur, które nie są już używane. Mapa `m_textures` nie rośnie w nieskończoność.
    -   **Wszystkie zalety Rozwiązania 2:** Proste i czyste API dla użytkownika.
-   **Wady:**
    -   **Bardziej złożona implementacja:** Logika wewnątrz `TextureManager` staje się bardziej skomplikowana (obsługa `lock()`, usuwanie nieaktualnych wpisów).
    -   **Potencjalny narzut wydajnościowy:** Operacje na `weak_ptr` mogą być nieco wolniejsze niż na `shared_ptr`, chociaż w typowej aplikacji GUI różnica będzie niezauważalna.
    -   **Zmiana zachowania:** Tekstura załadowana raz, a potem nieużywana, może zostać usunięta z pamięci. Przy ponownej próbie użycia (np. `getTexture`) trzeba by ją załadować od nowa (jeśli pochodziła z pliku), co może być nieoczekiwane.

## Podsumowanie i Rekomendacja

| Cecha | Rozwiązanie 1 (Jawne `shared_ptr`) | Rozwiązanie 2 (Przejęcie wskaźnika) | Rozwiązanie 3 (`weak_ptr`) |
| :--- | :--- | :--- | :--- |
| **API Użytkownika** | Złożone, wymaga wiedzy o `shared_ptr` | **Proste i intuicyjne** | Proste i intuicyjne |
| **Ryzyko błędu** | Wysokie (błędny deleter, wycieki) | Niskie | Niskie |
| **Zarządzanie cache**| Ręczne (wpisy pozostają na zawsze) | Ręczne (wpisy pozostają na zawsze) | **Automatyczne** |
| **Złożoność impl.** | Niska | Niska | Średnia |

**Rekomendacja:** **Rozwiązanie 2** jest najlepszym kompromisem między prostotą API, bezpieczeństwem a złożonością implementacji. Zapewnia doskonałe doświadczenie dla użytkownika biblioteki, jednocześnie będąc łatwe do wdrożenia i utrzymania. Rozwiązanie 3 jest technicznie eleganckie, ale może wprowadzać nieoczekiwane zachowanie i narzut, który nie jest konieczny na tym etapie.