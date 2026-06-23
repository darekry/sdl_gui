#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "panel.hpp"
#include "button.hpp"
#include "label.hpp"
#include "animation_manager.hpp"
#include "easing.hpp"
#include "theme.hpp"
#include "std.hpp"
// Panel that lifts upward on hover with an animated transition using AnimationManager
class HoverAnimatedLift : public Panel {
public:
    float m_anim_y_offset = 0.0f;
    float m_base_y = 0;
    float m_lift_amount = 15;
    uint32_t m_anim_duration = 200;
    uint32_t m_current_anim_id = 0;

    HoverAnimatedLift(GUIManager& manager, int x, int y, int width, int height)
        : Panel(manager, x, y, width, height), m_base_y(static_cast<float>(y)) {
        setBackgroundColor(ElementState::Normal, {100, 150, 200, 255});
        setBackgroundColor(ElementState::Hover, {120, 170, 220, 255});
        setBorderRadius(ElementState::Normal, 8);
    }

    bool handleEvent(const SDL_Event& event) override {
        if (!m_visible) return false;
        for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
            if ((*it)->handleEvent(event)) return true;
        }
        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            bool currentlyHovered = contains(event.motion.x, event.motion.y);
            if (currentlyHovered && !m_isHovered) {
                m_isHovered = true;
                setState(ElementState::Hover);
                startLiftAnimation();
            } else if (!currentlyHovered && m_isHovered) {
                m_isHovered = false;
                setState(ElementState::Normal);
                startDropAnimation();
            }
        }
        GUIElement::handleEvent(event);
        return false;
    }
    void startLiftAnimation() {
        if (m_current_anim_id != 0)
            m_manager.getAnimationManager()->removeAnimation(m_current_anim_id);
        m_manager.getAnimationManager()->createAnimation(
            &m_anim_y_offset, m_anim_y_offset,
            static_cast<float>(m_lift_amount), m_anim_duration,
            Easing::easeOutQuad);
        m_current_anim_id = m_manager.getAnimationManager()->addAnimation(16, [this]() {
            m_y = static_cast<int>(m_base_y - m_anim_y_offset);
            markDirty();
        });
    }

    void startDropAnimation() {
        if (m_current_anim_id != 0)
            m_manager.getAnimationManager()->removeAnimation(m_current_anim_id);
        m_manager.getAnimationManager()->createAnimation(
            &m_anim_y_offset, m_anim_y_offset, 0.0f, m_anim_duration,
            Easing::easeOutQuad,
            [this]() {
                if (m_current_anim_id != 0) {
                    m_manager.getAnimationManager()->removeAnimation(m_current_anim_id);
                    m_current_anim_id = 0;
                }
            });
    }
};
// Panel that lifts upward on hover instantly — no animation, for comparison
class HoverStaticLift : public Panel {
public:
    float m_base_y = 0;
    float m_lift_amount = 15;

    HoverStaticLift(GUIManager& manager, int x, int y, int width, int height)
        : Panel(manager, x, y, width, height), m_base_y(static_cast<float>(y)) {
        setBackgroundColor(ElementState::Normal, {100, 150, 200, 255});
        setBackgroundColor(ElementState::Hover, {120, 170, 220, 255});
        setBorderRadius(ElementState::Normal, 8);
    }

    bool handleEvent(const SDL_Event& event) override {
        if (!m_visible) return false;
        for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
            if ((*it)->handleEvent(event)) return true;
        }
        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            bool currentlyHovered = contains(event.motion.x, event.motion.y);
            if (currentlyHovered && !m_isHovered) {
                m_isHovered = true;
                setState(ElementState::Hover);
                m_y = m_base_y - m_lift_amount;
                markDirty();
            } else if (!currentlyHovered && m_isHovered) {
                m_isHovered = false;
                setState(ElementState::Normal);
                m_y = m_base_y;
                markDirty();
            }
        }
        GUIElement::handleEvent(event);
        return false;
    }
};
int main() {
    try {
        SDLApp app("Hover Animation Example", 600, 400);
        GUIManager gui(app.getRenderer());
        gui.setTheme(Theme::createDefaultTheme());
        auto title = std::make_unique<Label>(gui, 200, 20, "Hover Animation Demo");
        gui.addElement(std::move(title));

        // Animated lift — uses AnimationManager for smooth transition
        auto animLabel = std::make_unique<Label>(gui, 50, 80, "Animated Lift (smooth):");
        gui.addElement(std::move(animLabel));

        auto animPanel = std::make_unique<HoverAnimatedLift>(gui, 50, 120, 200, 100);
        auto* animPanelPtr = gui.addElement(std::move(animPanel));
        auto animInner = std::make_unique<Label>(gui, 50, 35, "Hover me!");
        animInner->setTextColor(ElementState::Normal, {255, 255, 255, 255});
        animPanelPtr->addChild(std::move(animInner));

        // Static lift — instant position change, no AnimationManager
        auto staticLabel = std::make_unique<Label>(gui, 320, 80, "Static Lift (instant):");
        gui.addElement(std::move(staticLabel));

        auto staticPanel = std::make_unique<HoverStaticLift>(gui, 320, 120, 200, 100);
        auto* staticPanelPtr = gui.addElement(std::move(staticPanel));
        auto staticInner = std::make_unique<Label>(gui, 50, 35, "Hover me!");
        staticInner->setTextColor(ElementState::Normal, {255, 255, 255, 255});
        staticPanelPtr->addChild(std::move(staticInner));

        auto info = std::make_unique<Label>(gui, 50, 270,
            "Animated: AnimationManager + TimerManager for smooth transitions");
        gui.addElement(std::move(info));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                gui.processEvent(e);
            }
            gui.update();
            SDL_SetRenderDrawColor(app.getRenderer(), 240, 240, 240, 255);
            SDL_RenderClear(app.getRenderer());
            gui.render();
            SDL_RenderPresent(app.getRenderer());
            gui.cleanup();
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
