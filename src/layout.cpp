#include "layout.hpp"
#include "gui.hpp"

// === AnchorLayout ===

LayoutSize AnchorLayout::measure(GUIElement& container, LayoutConstraints constraints) {
    // Kotwice nie narzucają rozmiaru kontenera — kontener zachowuje własny
    // rozmiar w ramach ograniczeń (shrink tylko gdy ograniczenie mniejsze).
    int w = container.getWidth();
    int h = container.getHeight();
    if (constraints.maxWidth > 0) w = std::min(w, constraints.maxWidth);
    if (constraints.maxHeight > 0) h = std::min(h, constraints.maxHeight);
    return LayoutSize{w, h};
}

void AnchorLayout::place(GUIElement& child, int parentWidth, int parentHeight) {
    const Anchor& a = child.getAnchor();
    if (!a.hasAnyAnchor()) {
        return;
    }

    int x = child.getX();
    int y = child.getY();
    int w = child.getWidth();
    int h = child.getHeight();

    switch (a.h) {
        case HAnchor::None:
            break;
        case HAnchor::Left:
            x = a.left;
            break;
        case HAnchor::Center:
            x = (parentWidth - w) / 2;
            break;
        case HAnchor::Right:
            x = parentWidth - a.right - w;
            break;
        case HAnchor::Stretch: {
            x = a.left;
            w = parentWidth - a.left - a.right;
            if (w < 0) w = 0;
            break;
        }
    }

    switch (a.v) {
        case VAnchor::None:
            break;
        case VAnchor::Top:
            y = a.top;
            break;
        case VAnchor::Center:
            y = (parentHeight - h) / 2;
            break;
        case VAnchor::Bottom:
            y = parentHeight - a.bottom - h;
            break;
        case VAnchor::Stretch: {
            y = a.top;
            h = parentHeight - a.top - a.bottom;
            if (h < 0) h = 0;
            break;
        }
    }

    if (x != child.getX() || y != child.getY()) {
        child.setPosition(x, y);
    }
    if (w != child.getWidth() || h != child.getHeight()) {
        child.setSize(w, h);
    }
}

void AnchorLayout::arrange(GUIElement& container) {
    const int pw = container.getWidth();
    const int ph = container.getHeight();
    for (const auto& child : container.getChildren()) {
        place(*child, pw, ph);
        child->layoutChildren();
    }
}

// === DockLayout ===

DockLayout::DockLayout(int spacing, int padLeft, int padTop, int padRight, int padBottom)
    : m_spacing(spacing)
    , m_padLeft(padLeft)
    , m_padTop(padTop)
    , m_padRight(padRight)
    , m_padBottom(padBottom) {}

LayoutSize DockLayout::measure(GUIElement& container, LayoutConstraints constraints) {
    // Content size dla ScrollArea: suma w osiach dokowania + rozmiar Fill.
    // Top/Bottom rozciągają się na szerokość (cross-axis ignorowany — zawsze
    // pasują), Left/Right na wysokość; Fill to reszta (0 przy overflow).
    // Z Fill: wynik = rozmiar kontenera (brak overflow → brak scrolla).
    // Bez Fill: minimum na pasy. Spacing>0 to górne oszacowanie (bezpieczne).
    int sumLeftW = 0, sumRightW = 0, sumTopH = 0, sumBottomH = 0;
    int fillW = 0, fillH = 0;
    int dockedCount = 0;
    for (const auto& child : container.getChildren()) {
        if (!child->isVisible()) {
            continue;
        }
        switch (child->getDock()) {
            case Dock::Top:
                sumTopH += child->getHeight();
                ++dockedCount;
                break;
            case Dock::Bottom:
                sumBottomH += child->getHeight();
                ++dockedCount;
                break;
            case Dock::Left:
                sumLeftW += child->getWidth();
                ++dockedCount;
                break;
            case Dock::Right:
                sumRightW += child->getWidth();
                ++dockedCount;
                break;
            case Dock::Fill:
                fillW = std::max(fillW, child->getWidth());
                fillH = std::max(fillH, child->getHeight());
                ++dockedCount;
                break;
            case Dock::None:
                break;
        }
    }
    int w = m_padLeft + m_padRight + sumLeftW + sumRightW + fillW;
    int h = m_padTop + m_padBottom + sumTopH + sumBottomH + fillH;
    if (dockedCount > 1) {
        w += m_spacing * (dockedCount - 1);
        h += m_spacing * (dockedCount - 1);
    }
    if (constraints.maxWidth > 0) w = std::min(w, constraints.maxWidth);
    if (constraints.maxHeight > 0) h = std::min(h, constraints.maxHeight);
    return LayoutSize{w, h};
}

void DockLayout::arrange(GUIElement& container) {
    const int cw = container.getWidth();
    const int ch = container.getHeight();

    int rx = m_padLeft;
    int ry = m_padTop;
    int rw = cw - m_padLeft - m_padRight;
    int rh = ch - m_padTop - m_padBottom;
    if (rw < 0) rw = 0;
    if (rh < 0) rh = 0;

    for (const auto& child : container.getChildren()) {
        if (!child->isVisible()) {
            continue;
        }
        const Dock d = child->getDock();
        if (d == Dock::None) {
            // Mieszane layouty: None zachowuje anchor (kompatybilność).
            const int wBefore = child->getWidth();
            const int hBefore = child->getHeight();
            AnchorLayout::place(*child, cw, ch);
            if (child->getWidth() == wBefore && child->getHeight() == hBefore) {
                child->layoutChildren();
            }
            // place() → setSize() już zrecurse'ował przy zmianie rozmiaru.
            continue;
        }

        int x = rx, y = ry, w = rw, h = rh;
        switch (d) {
            case Dock::Top:
                h = child->getHeight();
                w = rw;
                x = rx;
                y = ry;
                break;
            case Dock::Bottom:
                h = child->getHeight();
                w = rw;
                x = rx;
                y = ry + rh - h;
                break;
            case Dock::Left:
                w = child->getWidth();
                h = rh;
                x = rx;
                y = ry;
                break;
            case Dock::Right:
                w = child->getWidth();
                h = rh;
                x = rx + rw - w;
                y = ry;
                break;
            case Dock::Fill:
                x = rx;
                y = ry;
                w = rw;
                h = rh;
                break;
            case Dock::None:
                break;
        }
        if (w < 0) w = 0;
        if (h < 0) h = 0;

        const int wBefore = child->getWidth();
        const int hBefore = child->getHeight();
        if (x != child->getX() || y != child->getY()) {
            child->setPosition(x, y);
        }
        if (w != wBefore || h != hBefore) {
            child->setSize(w, h);
            // setSize() już woła layoutChildren() przy zmianie rozmiaru.
        } else {
            child->layoutChildren();
        }

        // Zjedz pas + spacing (bezwarunkowo — po ostatnim brak następcy,
        // więc mniejsza reszta nikomu nie szkodzi; zero alokacji/lookahead).
        switch (d) {
            case Dock::Top:
                ry += h + m_spacing;
                rh -= h + m_spacing;
                break;
            case Dock::Bottom:
                rh -= h + m_spacing;
                break;
            case Dock::Left:
                rx += w + m_spacing;
                rw -= w + m_spacing;
                break;
            case Dock::Right:
                rw -= w + m_spacing;
                break;
            case Dock::Fill:
                rw = 0;
                rh = 0;
                break;
            case Dock::None:
                break;
        }
        if (rw < 0) rw = 0;
        if (rh < 0) rh = 0;
    }
}

// === StackLayout ===

StackLayout::StackLayout(Direction dir, int spacing,
                         int padLeft, int padTop, int padRight, int padBottom,
                         Align align)
    : m_dir(dir)
    , m_spacing(spacing)
    , m_padLeft(padLeft)
    , m_padTop(padTop)
    , m_padRight(padRight)
    , m_padBottom(padBottom)
    , m_align(align) {}

LayoutSize StackLayout::measure(GUIElement& container, LayoutConstraints constraints) {
    // Suma rozmiarów dzieci + padding/spacing (content size, np. dla ScrollArea).
    int w = m_padLeft + m_padRight;
    int h = m_padTop + m_padBottom;
    const auto& children = container.getChildren();
    bool first = true;
    for (const auto& child : children) {
        if (!child->isVisible()) {
            continue;
        }
        if (!first) {
            if (m_dir == Direction::Vertical) h += m_spacing;
            else w += m_spacing;
        }
        first = false;
        if (m_dir == Direction::Vertical) {
            h += child->getHeight();
            w = std::max(w, m_padLeft + child->getWidth() + m_padRight);
        } else {
            w += child->getWidth();
            h = std::max(h, m_padTop + child->getHeight() + m_padBottom);
        }
    }
    if (constraints.maxWidth > 0) w = std::min(w, constraints.maxWidth);
    if (constraints.maxHeight > 0) h = std::min(h, constraints.maxHeight);
    return LayoutSize{w, h};
}

void StackLayout::arrange(GUIElement& container) {
    const int cw = container.getWidth();
    const int ch = container.getHeight();

    if (m_dir == Direction::Vertical) {
        int totalH = -m_spacing;
        for (const auto& c : container.getChildren()) {
            if (c->isVisible()) totalH += c->getHeight() + m_spacing;
        }
        int y = m_padTop;
        if (m_align == Align::Center) y = m_padTop + (ch - m_padTop - m_padBottom - totalH) / 2;
        else if (m_align == Align::End) y = ch - m_padBottom - totalH;
        for (const auto& c : container.getChildren()) {
            if (!c->isVisible()) continue;
            int x = m_padLeft;
            if (m_align == Align::Center) x = m_padLeft + (cw - m_padLeft - m_padRight - c->getWidth()) / 2;
            else if (m_align == Align::End) x = cw - m_padRight - c->getWidth();
            if (x != c->getX() || y != c->getY()) c->setPosition(x, y);
            y += c->getHeight() + m_spacing;
            c->layoutChildren();
        }
    } else {
        int totalW = -m_spacing;
        for (const auto& c : container.getChildren()) {
            if (c->isVisible()) totalW += c->getWidth() + m_spacing;
        }
        int x = m_padLeft;
        if (m_align == Align::Center) x = m_padLeft + (cw - m_padLeft - m_padRight - totalW) / 2;
        else if (m_align == Align::End) x = cw - m_padRight - totalW;
        for (const auto& c : container.getChildren()) {
            if (!c->isVisible()) continue;
            int y = m_padTop;
            if (m_align == Align::Center) y = m_padTop + (ch - m_padTop - m_padBottom - c->getHeight()) / 2;
            else if (m_align == Align::End) y = ch - m_padBottom - c->getHeight();
            if (x != c->getX() || y != c->getY()) c->setPosition(x, y);
            x += c->getWidth() + m_spacing;
            c->layoutChildren();
        }
    }
}

void StackLayout::arrangeStrip(const std::vector<GUIElement*>& items, int containerWidth, int y) const {
    int totalW = -m_spacing;
    for (const auto* it : items) {
        totalW += it->getWidth() + m_spacing;
    }
    int x = m_padLeft;
    if (m_align == Align::Center) x = m_padLeft + (containerWidth - m_padLeft - m_padRight - totalW) / 2;
    else if (m_align == Align::End) x = containerWidth - m_padRight - totalW;
    for (auto* it : items) {
        it->setPosition(x, y);
        x += it->getWidth() + m_spacing;
    }
}


