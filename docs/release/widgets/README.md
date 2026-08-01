# Widgety

Kompletna lista widgetów biblioteki SDL GUI. Każdy widget dziedziczy po
`GUIElement` (wspólne API — patrz [core.md](../core.md)), a te oznaczone
„(Panel)" dodatkowo po `Panel` (kontener, dzieci, przeciąganie).

## Podstawowe

| Widget | Opis |
|--------|------|
| [Button](Button.md) | Przycisk z obsługą kliknięcia, hover i fokusu klawiatury (Enter/Space) |
| [Checkbox](Checkbox.md) | Przełącznik tak/nie z fokusem klawiatury (Space) |
| [Label](Label.md) | Statyczny tekst; może być dzieckiem dowolnego widgetu |
| [Panel](Panel.md) | Kontener na inne elementy; opcjonalnie przeciągalny |
| [ProgressBar](ProgressBar.md) | Pasek postępu (poziomy/pionowy), opcjonalny tekst procentowy |
| [RadioButton](RadioButton.md) | Pojedynczy przycisk opcji (do użycia z RadioGroup) |
| [RadioGroup](RadioGroup.md) | (Panel) Grupa opcji wyboru jednej z wielu |

## Suwaki

| Widget | Opis |
|--------|------|
| [Slider](Slider.md) | (Panel) Suwak jednej wartości; przeciąganie, klik, kółko myszy |
| [RangeSlider](RangeSlider.md) | (Panel) Suwak zakresu (dolna/górna wartość), np. filtr cen |

## Tekst

| Widget | Opis |
|--------|------|
| [TextEditable](TextEditable.md) | Baza edycji tekstu: selekcja, clipboard, kursor (nie używać bezpośrednio) |
| [TextInput](TextInput.md) | Jednolinijkowe pole tekstowe; Enter, blokada, fokus |
| [TextArea](TextArea.md) | Wielolinijkowy edytor tekstu z zawijaniem wierszy |

## Dane i listy

| Widget | Opis |
|--------|------|
| [StringGrid](StringGrid.md) | (Panel) Tabela komórek tekstowych: sortowanie, selekcja, edycja |
| [ListView](ListView.md) | (StringGrid) Prosta lista wierszy z klik/aktywacją |
| [ComboBox](ComboBox.md) | Rozwijana lista wyboru jednej opcji |
| [TabControl](TabControl.md) | (Panel) Zakładki z osobnym panelem zawartości per tab |
| [ScrollArea](ScrollArea.md) | (Panel) Przewijany obszar z dowolną zawartością |

## Media i specjalne

| Widget | Opis |
|--------|------|
| [AnimatedImage](AnimatedImage.md) | Animacje z sprite sheet; play/pause/stop, skalowanie |
| [Canvas](Canvas.md) | Rysowanie odręczne myszą (pen color, clear) |
| [Cursor](Cursor.md) | Własny kursor myszy (statyczny/animowany, stany, hotspot) |
| [ArcContainer](ArcContainer.md) | Układa dzieci na łuku (np. menu kołowe, radary) |
| [ShaderPanel](ShaderPanel.md) | Panel z shaderem fragmentowym (wymaga GPU/Vulkan) |
| [ContextMenu](ContextMenu.md) | Menu kontekstowe po kliknięciu prawym przyciskiem |

## Kompozyty (gotowe dialogi)

[composites.md](../composites.md) — DialogBox, MessageBox, FileDialog.
