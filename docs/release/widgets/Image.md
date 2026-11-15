# Image

Minimalny widget obrazka: cienka nakładka na teksturę SDL ładowaną i
cache'owaną przez `TextureManager`. Bez animacji, edycji i obsługi myszy —
służy do wyświetlania statycznych grafik: portretów, ikon, podglądów,
teł paneli i arkuszy z zasobów gry.

## Przeznaczenie

`Image` trzyma `SharedTexture` (współdzieloną z cache'em `TextureManager`),
więc ta sama ścieżka użyta w wielu widgetach ładuje się raz. Rysowanie
przechodzi przez wspólny cache renderowania: klucz zawiera wskaźnik tekstury,
tryb skalowania i próbkowanie, więc identyczne obrazki tego samego rozmiaru
dzielą jedną teksturę cache'u.

## Tworzenie

```cpp
Image(GUIManager& manager, int x, int y, int width, int height);
```

```cpp
auto logo = std::make_unique<Image>(manager, 20, 20, 320, 180);
logo->setImage("assets/logo.png");

auto box = manager.makeRef(logo.get());
manager.addElement(std::move(logo));
```

## Źródła tekstury

| Metoda | Opis |
|--------|------|
| `bool setImage(const std::string& path)` | Ładuje przez cache ścieżek `TextureManager`. Zwraca `false` przy pustej ścieżce lub błędzie ładowania (czyści obrazek) |
| `bool setImageFromMemory(const uint8_t* data, size_t size, std::string_view key)` | Dekoduje z pamięci (assety osadzone) i cache'uje pod `key`. Klucz musi być niepusty |
| `void setImage(SharedTexture texture)` | Przyjmuje istniejącą teksturę (np. z `TextureManager::addTexture`); brak kopii |
| `void clearImage()` | Usuwa teksturę (idempotentne) |

## Geometria i skalowanie

| Metoda | Opis |
|--------|------|
| `void setScaleMode(ScaleMode mode)` | Sposób dopasowania tekstury do recta widgetu (patrz niżej) |
| `ScaleMode getScaleMode() const` | Bieżący tryb (domyślnie `Fit`) |
| `void setNearestScaling(bool nearest)` | Próbkowanie nearest (pixel art) zamiast linear. Uwaga: tryb jest właściwością tekstury SDL, więc dotyczy wszystkich widgetów dzielących tę teksturę |
| `bool isNearestScaling() const` | Czy nearest jest włączony |
| `void sizeToImage()` | Ustawia rozmiar widgetu na natywne wymiary tekstury (no-op bez obrazka) |
| `std::pair<int,int> getImageSize() const` | Wymiary tekstury; `{0,0}` bez obrazka |
| `bool hasImage() const` | Czy tekstura jest ustawiona |
| `SharedTexture getTexture() const` | Uchwyt tekstury (współdzielony) |
| `const std::string& getImagePath() const` | Ścieżka z `setImage(path)`; pusta dla pamięci/tekstury |

`ScaleMode`:

| Wartość | Zachowanie |
|---------|------------|
| `Fit` (domyślny) | Zmieść w rect z zachowaniem proporcji, wyśrodkuj (contain) |
| `Stretch` | Wypełnij cały rect, proporcje zignorowane |
| `Center` | Natywny rozmiar, wyśrodkowany; większy obrazek jest przycinany |
| `None` | Natywny rozmiar, lewy górny róg |

Tło i obramowanie pochodzą ze zwykłego `Style` (domyślnie przezroczyste we
wszystkich presetach). `borderRadius` nie przycina tekstury — do przyciętych
rozkładówek użyj `Panel`/`ArcContainer`.

## Przykład

```cpp
auto icon = std::make_unique<Image>(manager, 12, 12, 64, 64);
icon->setImage("assets/unit.png");
icon->setScaleMode(Image::ScaleMode::Center);
icon->setNearestScaling(true);   // pixel art bez rozmycia
manager.addElement(std::move(icon));
```

Pełny przykład: `examples/69_image.cpp`.
