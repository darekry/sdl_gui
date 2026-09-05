#pragma once

#include "style.hpp"
#include "component_type.hpp"

#include "std.hpp"

// StyleResolver (refactor plan §3):
// - ComponentType enum is the ONLY type key. String names exist solely at
//   the outer boundary (layout files via componentTypeFromString,
//   C-API via componentTypeToString).
// - O(1) array indexed by ComponentType; every mutation bumps epoch().
//   GUIElement caches the merged (local + theme + default) Style per state
//   and re-merges only when the theme epoch or its own local epoch changes.
// - The global shared render cache (TextureManager::m_renderCache) is keyed
//   by an integer hash including the type ID: identical widgets share ONE
//   cache entry / ONE texture (e.g. 100 RTS unit cards -> 1 entry).
class Theme {
public:
    void setStyle(ComponentType type, ElementState state, Style style);
    Style getStyle(ComponentType type, ElementState state) const;

    void setStyle(ComponentType type, Style style);
    Style getStyle(ComponentType type) const;

    void setDefaultStyle(Style style);
    const Style& getDefaultStyle() const;

    // Version counter for resolved-style caches. Bumped on every mutation.
    [[nodiscard]] uint64_t epoch() const { return m_epoch; }

    static Theme createDefaultTheme();
    static Theme createWindows95Theme();

private:
    static constexpr size_t stateIdx(ElementState s) { return static_cast<size_t>(s); }
    static constexpr size_t typeIdx(ComponentType t) { return static_cast<size_t>(t); }
    static constexpr size_t kNumTypes = static_cast<size_t>(ComponentType::Count);
    std::array<std::array<std::optional<Style>, 4>, kNumTypes> m_styles{};
    Style m_defaultStyle;
    uint64_t m_epoch = 1;
};
