# User Texture Management Proposal
[This page is also available in Polish](../../pl/archive/custom_texture_management_proposal.md)
[Back to Archive](./README.md)

Date: 2025-07-06
## Introduction

The library should allow users to create their own procedurally generated textures (e.g., icons, gradients, custom backgrounds) and register them in the TextureManager. The manager must correctly manage their lifetime, freeing memory when textures are no longer used.

See core files: [src/texture_manager.hpp](../../src/texture_manager.hpp:15), [src/sdl_deleters.hpp](../../src/sdl_deleters.hpp:1).

## Problem statement

1. Adding textures: How can a user add an existing `SDL_Texture*` into the `TextureManager` under a unique key (ID)?
2. Lifetime management: How to ensure the texture is automatically destroyed (via `SDL_DestroyTexture`) when the last GUI element stops using it?
3. API consistency: How to design the API so it is intuitive and consistent with existing methods like `loadTexture`?

---

## Solution 1: Explicit SharedTexture submission

In this approach, the user is responsible for wrapping the raw `SDL_Texture*` into a `std::shared_ptr` with an appropriate deleter (`SDLTextureDeleter`). Then they pass the smart pointer to the manager.

### Proposed API changes

```cpp
// In TextureManager
class TextureManager {
public:
    // ... existing methods ...

    // New method accepting an existing shared_ptr
    void addTexture(const std::string& key, SharedTexture texture);

    // Retrieve texture by key
    SharedTexture getTexture(const std::string& key);
};
```

### Usage example

```cpp
// User creates a texture
SDL_Texture* rawTexture = createMyCustomTexture(renderer); // user-defined function

// Wrap in shared_ptr with deleter
SharedTexture customTexture(rawTexture, SDLTextureDeleter());

// Add to the manager
textureManager.addTexture("close_icon", customTexture);

// ...

// GUI element retrieves and uses the texture
auto buttonTexture = textureManager.getTexture("close_icon");
closeButton->setTexture(buttonTexture);
```

### Analysis

- Pros:
  - Explicitness and control: the user fully controls how `shared_ptr` is created and that ownership is transferred.
  - Simple manager implementation: the manager only stores the provided `shared_ptr`.
- Cons:
  - More work on the user: must remember to set the correct deleter; error-prone.
  - Less friendly API: requires knowledge of `SharedTexture` and the deleter.

---

## Solution 2: Take ownership of the raw pointer (Recommended)

In this approach, the user passes a raw `SDL_Texture*` to the manager, and the manager takes responsibility for wrapping it into a `std::shared_ptr` and managing its lifetime.

### Proposed API changes

```cpp
// In TextureManager
class TextureManager {
public:
    // ... existing methods ...

    // New method that takes ownership of the raw pointer
    // Returns SharedTexture so the caller can use it immediately.
    SharedTexture addTexture(const std::string& key, SDL_Texture* texture);

    // Retrieve texture by key (unchanged)
    SharedTexture getTexture(const std::string& key);
};
```

### Implementation sketch in TextureManager

```cpp
SharedTexture TextureManager::addTexture(const std::string& key, SDL_Texture* texture) {
    if (m_textures.count(key)) {
        // Key already exists; return existing texture to avoid a leak
        return m_textures[key];
    }
    if (!texture) {
        return nullptr;
    }
    SharedTexture shared(texture, SDLTextureDeleter());
    m_textures[key] = shared;
    return shared;
}
```

### Usage example

```cpp
// User creates a texture
SDL_Texture* rawTexture = createMyCustomTexture(renderer);

// Transfer ownership to the manager; use returned shared_ptr right away
SharedTexture buttonTexture = textureManager.addTexture("close_icon", rawTexture);
closeButton->setTexture(buttonTexture);

// The user does not need to free rawTexture anymore.
```

### Analysis

- Pros:
  - Simple and clean API: the user does not need to know about `std::shared_ptr` or deleters.
  - Clear contract: passing a raw pointer to `addTexture` clearly signals ownership transfer.
  - Lower error risk: fewer chances to forget a deleter or create an incorrect `shared_ptr`.
- Cons:
  - Some "magic" inside the manager is required, but this is standard practice in well-designed libraries.

---

## Solution 3: Use std::weak_ptr for automatic cache cleanup

This approach extends Solution 2. The `TextureManager` stores `std::weak_ptr` instead of `std::shared_ptr`. That allows the manager to detect when a texture is no longer used (the `shared_ptr` reference count drops to zero) and remove it from the internal cache.

### Proposed API changes

The user-facing API remains identical to Solution 2. Only the internal implementation changes.

### Implementation sketch in TextureManager

```cpp
class TextureManager {
public:
    // ... API unchanged ...

private:
    SDL_Renderer* m_renderer;
    // The map now holds weak_ptr
    std::map<std::string, std::weak_ptr<SDL_Texture>> m_textures;
};

// getTexture (or loadTexture) must be modified
SharedTexture TextureManager::getTexture(const std::string& key) {
    if (m_textures.count(key)) {
        // Try to "lock" weak_ptr to get a shared_ptr
        SharedTexture texture = m_textures[key].lock();
        if (texture) {
            // Still alive
            return texture;
        } else {
            // Expired; remove stale entry
            m_textures.erase(key);
        }
    }
    // If missing or stale, either load from file (if this is loadTexture) or return nullptr.
    return nullptr;
}

SharedTexture TextureManager::addTexture(const std::string& key, SDL_Texture* texture) {
    // ... (check key existence) ...
    SharedTexture shared(texture, SDLTextureDeleter());
    m_textures[key] = shared; // Stores weak_ptr in the map
    return shared;
}
```

### Analysis

- Pros:
  - Automatic cache management: the manager does not keep unused textures alive; the map does not grow forever.
  - Retains all advantages of Solution 2: simple, clean user API.
- Cons:
  - More complex implementation: handling `lock()` and removing stale entries.
  - Potential minor performance overhead.
  - Behavioral change: once unused, a texture can be evicted; re-acquiring may require reloading (if it came from a file), which might be unexpected.

## Summary and Recommendation

| Aspect | Solution 1 (explicit shared_ptr) | Solution 2 (take ownership) | Solution 3 (weak_ptr) |
| :--- | :--- | :--- | :--- |
| User API | Complex, requires shared_ptr knowledge | Simple and intuitive | Simple and intuitive |
| Error risk | High (wrong/missing deleter, leaks) | Low | Low |
| Cache management | Manual (entries remain forever) | Manual (entries remain forever) | Automatic |
| Implementation complexity | Low | Low | Medium |

Recommendation: Solution 2 is the best compromise between API simplicity, safety, and implementation effort. It provides a great developer experience, aligns with the design around [src/texture_manager.hpp](../../src/texture_manager.hpp:15), and is straightforward to implement and maintain. Solution 3 is technically elegant but may introduce unexpected behavior and overhead that is not necessary at this stage.