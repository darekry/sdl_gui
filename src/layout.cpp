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


