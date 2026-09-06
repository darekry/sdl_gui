#pragma once

#include "std.hpp"

// ElementHandle — generational weak reference to a GUIElement (point 5).
//
// Replaces raw-pointer + m_liveElements membership tests. Each registration
// gets a slot {index, generation}; unregister bumps the generation, so a
// stale handle NEVER resolves — even if the freed address is reused by a new
// element (classic ABA that unordered_set<raw*> cannot catch).
//
// Handles are cheap to copy and safe to hold in lambdas, focus/capture
// slots and editor maps. Resolution is O(1) through GUIManager.

struct ElementHandle {
    static constexpr uint32_t kInvalidIndex = 0xFFFFFFFFu;

    uint32_t index = kInvalidIndex;
    uint32_t generation = 0;

    constexpr ElementHandle() = default;
    constexpr ElementHandle(uint32_t idx, uint32_t gen) : index(idx), generation(gen) {}

    [[nodiscard]] constexpr bool valid() const { return index != kInvalidIndex; }
    constexpr void reset() { index = kInvalidIndex; generation = 0; }

    constexpr bool operator==(const ElementHandle& o) const {
        return index == o.index && generation == o.generation;
    }
    constexpr bool operator!=(const ElementHandle& o) const { return !(*this == o); }
};

struct ElementHandleHash {
    size_t operator()(const ElementHandle& h) const noexcept {
        return (static_cast<size_t>(h.index) << 32) ^ static_cast<size_t>(h.generation);
    }
};
