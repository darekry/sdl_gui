#pragma once

#include "std.hpp"

class GUIElement;

/**
 * @brief OverlayStack (punkt 6, plaster 1): jedno źródło prawdy o modalności.
 *
 * GUIManager trzyma WSZYSTKIE top-level elementy w jednym wektorze
 * (stabilna kolejność Z, brak przenoszenia ownershipu). OverlayStack to
 * lekki cache widoków (non-owning) na elementy z isOverlay()==true.
 *
 * Dlaczego cache, a nie osobny magazyn: StringGrid::isOverlay() jest
 * dynamiczne (m_isEditing) — przenoszenie unique_ptr między wektorami przy
 * każdym start/stopEditing łamałoby kolejność Z i wymagało protokołu
 * powrotu. Cache unieważniany jawnie (add/detach/cleanup +
 * notifyOverlayChanged() ze StringGrid) daje O(K) zamiast O(N) w ścieżkach
 * overlayowych (render-overlay pass, getActiveOverlay, focus) bez ruszania
 * ownershipu i bez zmiany kolejności eventów.
 */
class OverlayStack {
public:
    void markDirty() { m_dirty = true; }
    [[nodiscard]] bool isDirty() const { return m_dirty; }

    void rebuild(const std::vector<std::unique_ptr<GUIElement>>& elements);

    [[nodiscard]] const std::vector<GUIElement*>& overlays() const { return m_cached; }

    /// Topmost (ostatni dodany) widoczny i nieusunięty overlay, albo nullptr.
    [[nodiscard]] GUIElement* topActive() const;

private:
    std::vector<GUIElement*> m_cached;
    bool m_dirty = true;
};
