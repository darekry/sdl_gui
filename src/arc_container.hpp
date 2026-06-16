#pragma once
#include "gui.hpp"

class ArcContainer : public GUIElement {
public:
    ArcContainer(GUIManager& manager, int centerX, int centerY, int radius,
                 float startAngleDeg = 0.0f, float endAngleDeg = 360.0f);
    
    bool contains(int x, int y) const override;
    bool contains(float x, float y) const { return contains(static_cast<int>(x), static_cast<int>(y)); }
    
    void addChildAtAngle(std::unique_ptr<GUIElement> child, float angleDeg, 
                         bool rotateChild = true, int offset = 0);
    
    void setRadius(int radius);
    void setArcRange(float startAngleDeg, float endAngleDeg);
    
    const char* getComponentType() const override;
    
protected:
    void draw(SDL_Renderer*) override {}
    
private:
    SDL_Point m_center;
    int m_radius;
    float m_startAngle;
    float m_endAngle;
    int m_innerRadius;
    
    static float normalizeAngle(float angle);
    static bool angleInRange(float angle, float start, float end);
};