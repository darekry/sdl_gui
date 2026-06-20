#include "sdl_app.hpp"
#include "gui_manager.hpp"
#include "panel.hpp"
#include "button.hpp"
#include "label.hpp"
#include "animation_manager.hpp"
#include "easing.hpp"

#include "std.hpp"

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
        if (m_current_anim_id != 0) {
            m_manager.getAnimationManager()->removeAnimation(m_current_anim_id);
        }
        m_manager.getAnimationManager()->createAnimation(
            &m_anim_y_offset,
            m_anim_y_offset,
            static_cast<float>(m_lift_amount),
            m_anim_duration,
            Easing::easeOutQuad
        );
        m_current_anim_id = m_manager.getAnimationManager()->addAnimation(16, [this]() {
            m_y = static_cast<int>(m_base_y - m_anim_y_offset);
            markDirty();
        });
    }

    void startDropAnimation() {
        if (m_current_anim_id != 0) {
            m_manager.getAnimationManager()->removeAnimation(m_current_anim_id);
        }
        m_manager.getAnimationManager()->createAnimation(
            &m_anim_y_offset,
            m_anim_y_offset,
            0.0f,
            m_anim_duration,
            Easing::easeOutQuad,
            [this]() {
                if (m_current_anim_id != 0) {
                    m_manager.getAnimationManager()->removeAnimation(m_current_anim_id);
                    m_current_anim_id = 0;
                }
            }
        );
    }
};

class HoverAnimatedScale : public Panel {
public:
    float m_anim_scale = 1.0f;
    float m_base_width = 0;
    float m_base_height = 0;
    float m_scale_factor = 1.15f;
    uint32_t m_anim_duration = 200;
    uint32_t m_current_anim_id = 0;

    HoverAnimatedScale(GUIManager& manager, int x, int y, int width, int height)
        : Panel(manager, x, y, width, height), m_base_width(static_cast<float>(width)), m_base_height(static_cast<float>(height)) {
        setBackgroundColor(ElementState::Normal, {200, 100, 150, 255});
        setBackgroundColor(ElementState::Hover, {220, 120, 170, 255});
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
                startScaleUpAnimation();
            } else if (!currentlyHovered && m_isHovered) {
                m_isHovered = false;
                setState(ElementState::Normal);
                startScaleDownAnimation();
            }
        }

        GUIElement::handleEvent(event);
        return false;
    }

    void startScaleUpAnimation() {
        if (m_current_anim_id != 0) {
            m_manager.getAnimationManager()->removeAnimation(m_current_anim_id);
        }
        m_manager.getAnimationManager()->createAnimation(
            &m_anim_scale,
            m_anim_scale,
            m_scale_factor,
            m_anim_duration,
            Easing::easeOutQuad
        );
        m_current_anim_id = m_manager.getAnimationManager()->addAnimation(16, [this]() {
            m_width = static_cast<int>(m_base_width * m_anim_scale);
            m_height = static_cast<int>(m_base_height * m_anim_scale);
            markDirty();
        });
    }

    void startScaleDownAnimation() {
        if (m_current_anim_id != 0) {
            m_manager.getAnimationManager()->removeAnimation(m_current_anim_id);
        }
        m_manager.getAnimationManager()->createAnimation(
            &m_anim_scale,
            m_anim_scale,
            1.0f,
            m_anim_duration,
            Easing::easeOutQuad,
            [this]() {
                if (m_current_anim_id != 0) {
                    m_manager.getAnimationManager()->removeAnimation(m_current_anim_id);
                    m_current_anim_id = 0;
                }
            }
        );
    }
};

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

class HoverStaticScale : public Panel {
public:
    float m_base_width = 0;
    float m_base_height = 0;
    float m_scale_factor = 1.15f;

    HoverStaticScale(GUIManager& manager, int x, int y, int width, int height)
        : Panel(manager, x, y, width, height), m_base_width(static_cast<float>(width)), m_base_height(static_cast<float>(height)) {
        setBackgroundColor(ElementState::Normal, {200, 100, 150, 255});
        setBackgroundColor(ElementState::Hover, {220, 120, 170, 255});
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
                m_width = static_cast<int>(m_base_width * m_scale_factor);
                m_height = static_cast<int>(m_base_height * m_scale_factor);
                markDirty();
            } else if (!currentlyHovered && m_isHovered) {
                m_isHovered = false;
                setState(ElementState::Normal);
                m_width = static_cast<int>(m_base_width);
                m_height = static_cast<int>(m_base_height);
                markDirty();
            }
        }

        GUIElement::handleEvent(event);
        return false;
    }
};

int main() {
    try {
        SDLApp app("Hover Animation Example", 900, 600);
        GUIManager gui(app.getRenderer());

        auto title_label = std::make_unique<Label>(gui, 350, 20, "Hover Animation Demo");
        gui.addElement(std::move(title_label));

        auto anim_lift_label = std::make_unique<Label>(gui, 50, 100, "Animated Lift:");
        gui.addElement(std::move(anim_lift_label));

        auto anim_lift = std::make_unique<HoverAnimatedLift>(gui, 50, 140, 150, 100);
        auto* anim_lift_ptr = gui.addElement(std::move(anim_lift));
        auto anim_lift_inner = std::make_unique<Label>(gui, 30, 35, "Hover me!");
        anim_lift_inner->setTextColor(ElementState::Normal, {255, 255, 255, 255});
        anim_lift_ptr->addChild(std::move(anim_lift_inner));

        auto anim_scale_label = std::make_unique<Label>(gui, 250, 100, "Animated Scale:");
        gui.addElement(std::move(anim_scale_label));

        auto anim_scale = std::make_unique<HoverAnimatedScale>(gui, 250, 140, 150, 100);
        auto* anim_scale_ptr = gui.addElement(std::move(anim_scale));
        auto anim_scale_inner = std::make_unique<Label>(gui, 30, 35, "Hover me!");
        anim_scale_inner->setTextColor(ElementState::Normal, {255, 255, 255, 255});
        anim_scale_ptr->addChild(std::move(anim_scale_inner));

        auto static_lift_label = std::make_unique<Label>(gui, 500, 100, "Static Lift:");
        gui.addElement(std::move(static_lift_label));

        auto static_lift = std::make_unique<HoverStaticLift>(gui, 500, 140, 150, 100);
        auto* static_lift_ptr = gui.addElement(std::move(static_lift));
        auto static_lift_inner = std::make_unique<Label>(gui, 30, 35, "Hover me!");
        static_lift_inner->setTextColor(ElementState::Normal, {255, 255, 255, 255});
        static_lift_ptr->addChild(std::move(static_lift_inner));

        auto static_scale_label = std::make_unique<Label>(gui, 700, 100, "Static Scale:");
        gui.addElement(std::move(static_scale_label));

        auto static_scale = std::make_unique<HoverStaticScale>(gui, 700, 140, 150, 100);
        auto* static_scale_ptr = gui.addElement(std::move(static_scale));
        auto static_scale_inner = std::make_unique<Label>(gui, 30, 35, "Hover me!");
        static_scale_inner->setTextColor(ElementState::Normal, {255, 255, 255, 255});
        static_scale_ptr->addChild(std::move(static_scale_inner));

        auto info_label = std::make_unique<Label>(gui, 50, 300, 
            "Animated: AnimationManager + TimerManager for smooth transitions");
        gui.addElement(std::move(info_label));

        bool quit = false;
        SDL_Event e;
        while (!quit) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) quit = true;
                gui.processEvent(e);
            }

            gui.update();
            gui.cleanup();

            SDL_SetRenderDrawColor(app.getRenderer(), 240, 240, 240, 255);
            SDL_RenderClear(app.getRenderer());
            gui.render();
            SDL_RenderPresent(app.getRenderer());
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}