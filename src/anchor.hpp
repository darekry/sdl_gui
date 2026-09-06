#pragma once

#include "std.hpp"

/**
 * @brief Viewport — rozmiar obszaru layoutu (okna) z niezmiennikiem NonZero.
 *
 * GUIManager wymaga Viewport w konstruktorze, więc rozmiar okna nigdy nie
 * jest 0x0 i żaden kod nie potrzebuje fallbacku (dawne 800x600 w ContextMenu).
 * handleResize() ignoruje wymiary <= 0 (np. minimalizacja okna), zachowując
 * niezmiennik przez cały czas życia menedżera.
 */
struct Viewport {
    int width = 0;
    int height = 0;

    constexpr Viewport() = default;
    constexpr Viewport(int w, int h) : width(w), height(h) {}

    [[nodiscard]] constexpr bool valid() const { return width > 0 && height > 0; }
};

/**
 * @brief Pozioma kotwica elementu względem rodzica (enum, nie magiczne floaty).
 *
 * - None:    brak kotwicy na tej osi — pozycja/rozmiar bez zmian.
 * - Left:    lewa krawędź elementu `left` px od lewej krawędzi rodzica.
 * - Center:  element wycentrowany poziomo (środek w środku rodzica).
 * - Right:   prawa krawędź elementu `right` px od prawej krawędzi rodzica.
 * - Stretch: element rozciągnięty: x = left, w = parentW - left - right.
 *
 * Wszystkie marginesy to int w pikselach — 1px jest osiągalne (dawne 1.0
 * znaczyło 100% szerokości rodzica), a center to osobny wariant (dawne 0.5
 * znaczyło naraz 50% i "centruj").
 */
enum class HAnchor : uint8_t { None, Left, Center, Right, Stretch };

/**
 * @brief Pionowa kotwica elementu względem rodzica (enum, nie magiczne floaty).
 *
 * - None:   brak kotwicy na tej osi.
 * - Top:    górna krawędź `top` px od góry rodzica.
 * - Center: element wycentrowany pionowo.
 * - Bottom: dolna krawędź `bottom` px od dołu rodzica.
 * - Stretch: y = top, h = parentH - top - bottom.
 */
enum class VAnchor : uint8_t { None, Top, Center, Bottom, Stretch };

/**
 * @brief Anchor — deklaratywne pozycjonowanie elementu względem rodzica.
 *
 * Logika aranżacji mieszka w AnchorLayout (src/layout.hpp), tutaj tylko dane:
 * tryb per oś + marginesy w pikselach. Rozmiar elementu na osiach
 * nie-stretchowanych pochodzi z jego bieżącego rozmiaru (brak historii
 * m_originalW/H — center liczone z aktualnego rozmiaru, nigdy ze starych).
 *
 * Przykłady:
 * - Fixed top-left:      Anchor::at(10, 10)
 * - Centered:            Anchor::center()
 * - Bottom-right:        Anchor::pinned(HAnchor::Right, VAnchor::Bottom, 0, 0, 10, 10)
 * - Full stretch:        Anchor::fill(0)
 * - Horizontal bar:      Anchor::bottomBar(50, 10, 10)
 */
struct Anchor {
    HAnchor h = HAnchor::None;
    VAnchor v = VAnchor::None;
    int left = 0;    // px od lewej (Left/Stretch)
    int top = 0;     // px od góry (Top/Stretch)
    int right = 0;   // px od prawej (Right/Stretch)
    int bottom = 0;  // px od dołu (Bottom/Stretch)

    constexpr Anchor() = default;
    constexpr Anchor(HAnchor hh, VAnchor vv, int l = 0, int t = 0, int r = 0, int b = 0)
        : h(hh), v(vv), left(l), top(t), right(r), bottom(b) {}

    [[nodiscard]] constexpr bool hasAnyAnchor() const {
        return h != HAnchor::None || v != VAnchor::None;
    }
    [[nodiscard]] constexpr bool stretchesHorizontal() const { return h == HAnchor::Stretch; }
    [[nodiscard]] constexpr bool stretchesVertical() const { return v == VAnchor::Stretch; }

    // === Presety ===

    /** Brak kotwicy — stała pozycja i rozmiar. */
    static Anchor none() { return Anchor{}; }

    /** Dowolna kombinacja bez presetu (zastępuje dawny surowy init floatami). */
    static Anchor pinned(HAnchor hh, VAnchor vv, int l = 0, int t = 0, int r = 0, int b = 0) {
        return Anchor{hh, vv, l, t, r, b};
    }

    /** Stała pozycja (x, y) bez kotwicy rozciągającej. */
    static Anchor at(int x, int y) {
        return Anchor{HAnchor::Left, VAnchor::Top, x, y, 0, 0};
    }

    /** Kotwica do lewego górnego rogu z marginesem. */
    static Anchor topLeft(int margin = 0) {
        return Anchor{HAnchor::Left, VAnchor::Top, margin, margin, 0, 0};
    }

    /** Kotwica do prawego górnego rogu z marginesem. */
    static Anchor topRight(int margin = 0) {
        return Anchor{HAnchor::Right, VAnchor::Top, 0, margin, margin, 0};
    }

    /** Kotwica do lewego dolnego rogu z marginesem. */
    static Anchor bottomLeft(int margin = 0) {
        return Anchor{HAnchor::Left, VAnchor::Bottom, margin, 0, 0, margin};
    }

    /** Kotwica do prawego dolnego rogu z marginesem. */
    static Anchor bottomRight(int margin = 0) {
        return Anchor{HAnchor::Right, VAnchor::Bottom, 0, 0, margin, margin};
    }

    /** Kotwica do prawego dolnego rogu z osobnymi marginesami. */
    static Anchor bottomRightAt(int rightMargin, int bottomMargin) {
        return Anchor{HAnchor::Right, VAnchor::Bottom, 0, 0, rightMargin, bottomMargin};
    }

    /** Wycentrowanie w rodzicu (zachowuje rozmiar). */
    static Anchor center() {
        return Anchor{HAnchor::Center, VAnchor::Center};
    }

    /** Wycentrowanie poziome z marginesem od góry. */
    static Anchor topCenter(int topMargin = 0) {
        return Anchor{HAnchor::Center, VAnchor::Top, 0, topMargin, 0, 0};
    }

    /** Wypełnienie całego rodzica z jednolitym marginesem. */
    static Anchor fill(int margin = 0) {
        return Anchor{HAnchor::Stretch, VAnchor::Stretch, margin, margin, margin, margin};
    }

    /** Rozciągnięcie poziome (wysokość zachowana). */
    static Anchor horizontalStretch(int leftMargin = 0, int rightMargin = 0) {
        return Anchor{HAnchor::Stretch, VAnchor::None, leftMargin, 0, rightMargin, 0};
    }

    /** Rozciągnięcie pionowe (szerokość zachowana). */
    static Anchor verticalStretch(int topMargin = 0, int bottomMargin = 0) {
        return Anchor{HAnchor::None, VAnchor::Stretch, 0, topMargin, 0, bottomMargin};
    }

    /** Dolny pasek — pełna szerokość, dolna krawędź bottomMargin px od dołu. */
    static Anchor bottomBar(int bottomMargin, int leftMargin = 0, int rightMargin = 0) {
        return Anchor{HAnchor::Stretch, VAnchor::Bottom, leftMargin, 0, rightMargin, bottomMargin};
    }

    /** Górny pasek — pełna szerokość, górna krawędź topMargin px od góry. */
    static Anchor topBar(int topMargin, int leftMargin = 0, int rightMargin = 0) {
        return Anchor{HAnchor::Stretch, VAnchor::Top, leftMargin, topMargin, rightMargin, 0};
    }

    /** Lewy sidebar — pełna wysokość z marginesami góra/dół.
     *  Szerokość pochodzi z rozmiaru elementu (ustaw ją w konstruktorze). */
    static Anchor leftSidebar(int topMargin = 0, int bottomMargin = 0) {
        return Anchor{HAnchor::Left, VAnchor::Stretch, 0, topMargin, 0, bottomMargin};
    }

    /** Prawy sidebar — pełna wysokość z marginesami góra/dół.
     *  Szerokość pochodzi z rozmiaru elementu (ustaw ją w konstruktorze). */
    static Anchor rightSidebar(int topMargin = 0, int bottomMargin = 0) {
        return Anchor{HAnchor::Right, VAnchor::Stretch, 0, topMargin, 0, bottomMargin};
    }
};
