#include "overlay_stack.hpp"
#include "gui.hpp"

void OverlayStack::rebuild(const std::vector<std::unique_ptr<GUIElement>>& elements) {
    m_cached.clear();
    m_cached.reserve(elements.size() > 8 ? 8 : elements.size());
    for (const auto& e : elements) {
        if (e && e->isOverlay()) {
            m_cached.push_back(e.get());
        }
    }
    m_dirty = false;
}

GUIElement* OverlayStack::topActive() const {
    for (auto it = m_cached.rbegin(); it != m_cached.rend(); ++it) {
        GUIElement* e = *it;
        if (e && e->isVisible() && !e->isMarkedForDeletion()) {
            return e;
        }
    }
    return nullptr;
}
