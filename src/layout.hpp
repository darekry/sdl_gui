#pragma once

#include "anchor.hpp"
#include "std.hpp"

class GUIElement;

/**
 * @brief Layout pass (Measure/Arrange) — jeden silnik layoutu zamiast trzech
 * rozproszonych (Anchor::applyAnchor/updateLayout/onParentResize +
 * ScrollArea::updateLayout + DialogBox::layoutButtons).
 *
 * - measure() szacuje rozmiar kontenera przy danych ograniczeniach;
 * - arrange() ustawia finalne recty dzieci kontenera i rekurencyjnie
 *   wywołuje ich layoutChildren().
 *
 * Kontener wybiera menedżera przez GUIElement::setLayoutManager().
 * Domyślny (brak menedżera) to AnchorLayout — każde dziecko pozycjonowane
 * wg własnego Anchor. Widgety o własnej geometrii wewnętrznej (Button,
 * Slider, ScrollArea, TabControl, DialogBox, FileDialog) nadpisują
 * layoutChildren() zamiast dawnego hooka onSizeChanged().
 */
struct LayoutConstraints {
    int maxWidth = 0;
    int maxHeight = 0;
};

struct LayoutSize {
    int width = 0;
    int height = 0;
};

class ILayoutManager {
public:
    virtual ~ILayoutManager() = default;
    virtual LayoutSize measure(GUIElement& container, LayoutConstraints constraints) = 0;
    virtual void arrange(GUIElement& container) = 0;
};

/**
 * @brief AnchorLayout — domyślny menedżer: każde dziecko wg własnego Anchor.
 *
 * place() to cała matematyka kotwic (piksele int, Center jako wariant):
 * - Stretch:  pos = margines, rozmiar = parent - marginesy (clamp >= 0);
 * - Center:   pos = (parent - rozmiar) / 2 (rozmiar bieżący, bez historii);
 * - Left/Right/Top/Bottom: krawędź na marginesie, rozmiar zachowany;
 * - None na osi: pozycja i rozmiar bez zmian.
 */
class AnchorLayout : public ILayoutManager {
public:
    LayoutSize measure(GUIElement& container, LayoutConstraints constraints) override;
    void arrange(GUIElement& container) override;

    /** Ustawia rect jednego dziecka względem rozmiaru rodzica. */
    static void place(GUIElement& child, int parentWidth, int parentHeight);
};

/**
 * @brief StackLayout — liniowe układanie dzieci (pasek przycisków, kolumny).
 *
 * arrange() układa WSZYSTKIE dzieci kontenera w jednym kierunku.
 * arrangeStrip() układa podany podzbiór (np. przyciski DialogBox/FileDialog)
 * w poziomym pasie na wysokości y z wyrównaniem Align.
 */
class StackLayout : public ILayoutManager {
public:
    enum class Direction { Vertical, Horizontal };
    enum class Align { Start, Center, End };

    StackLayout(Direction dir = Direction::Vertical, int spacing = 8,
                int padLeft = 0, int padTop = 0, int padRight = 0, int padBottom = 0,
                Align align = Align::Start);

    LayoutSize measure(GUIElement& container, LayoutConstraints constraints) override;
    void arrange(GUIElement& container) override;

    /** Poziomy pas dla podzbioru dzieci (przyciski dialogów). */
    void arrangeStrip(const std::vector<GUIElement*>& items, int containerWidth, int y) const;

private:
    Direction m_dir;
    int m_spacing;
    int m_padLeft, m_padTop, m_padRight, m_padBottom;
    Align m_align;
};
