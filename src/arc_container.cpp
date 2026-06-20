#include "arc_container.hpp"
#define M_PI std::numbers::pi
#include "std.hpp"

ArcContainer::ArcContainer(GUIManager& manager, int centerX, int centerY, int radius,
                           float startAngleDeg, float endAngleDeg)
    : GUIElement(manager, centerX - radius, centerY - radius, radius * 2, radius * 2)
    , m_center{centerX, centerY}
    , m_radius(radius)
    , m_startAngle(startAngleDeg)
    , m_endAngle(endAngleDeg)
    , m_innerRadius(radius - 50) {
    m_clip_children = false;
}

float ArcContainer::normalizeAngle(float angle) {
    while (angle < 0) angle += 360.0f;
    while (angle >= 360) angle -= 360.0f;
    return angle;
}

bool ArcContainer::angleInRange(float angle, float start, float end) {
    angle = normalizeAngle(angle);
    start = normalizeAngle(start);
    end = normalizeAngle(end);
    
    if (start <= end) {
        return angle >= start && angle <= end;
    } else {
        return angle >= start || angle <= end;
    }
}

bool ArcContainer::contains(int gx, int gy) const {
    auto abs = getAbsolutePosition();
    
    float dx = static_cast<float>(gx - (abs.x + m_center.x - m_x));
    float dy = static_cast<float>(gy - (abs.y + m_center.y - m_y));
    float dist = std::sqrt(dx * dx + dy * dy);
    
    if (dist > static_cast<float>(m_radius) + 10.0f || dist < static_cast<float>(m_innerRadius) - 10.0f) {
        return false;
    }
    
    constexpr float kPiF = 3.14159265358979f;
    float angleDeg = std::atan2(dy, dx) * 180.0f / kPiF;
    angleDeg = normalizeAngle(angleDeg);
    
    return angleInRange(angleDeg, m_startAngle, m_endAngle);
}

void ArcContainer::addChildAtAngle(std::unique_ptr<GUIElement> child, float angleDeg,
                                    bool rotateChild, int offset) {
    constexpr float kPiF = 3.14159265358979f;
    float rad = angleDeg * kPiF / 180.0f;
    int effectiveRadius = m_radius + offset;
    
    int childW = child->m_width;
    int childH = child->m_height;
    
    float cx = static_cast<float>(m_center.x);
    float cy = static_cast<float>(m_center.y);
    float fr = static_cast<float>(effectiveRadius);
    float fchildW = static_cast<float>(childW);
    float fchildH = static_cast<float>(childH);
    
    int x = static_cast<int>(cx + std::cos(rad) * fr - fchildW * 0.5f);
    int y = static_cast<int>(cy + std::sin(rad) * fr - fchildH * 0.5f);
    
    child->setPosition(x - m_x, y - m_y);
    
    if (rotateChild) {
        child->setRotation(static_cast<double>(angleDeg) + 90.0);
    }
    
    GUIElement::addChild(std::move(child));
}

void ArcContainer::setRadius(int radius) {
    m_radius = radius;
    m_innerRadius = radius - 50;
    setPosition(m_center.x - radius, m_center.y - radius);
    setSize(radius * 2, radius * 2);
    markDirty();
}

void ArcContainer::setArcRange(float startAngleDeg, float endAngleDeg) {
    m_startAngle = startAngleDeg;
    m_endAngle = endAngleDeg;
    markDirty();
}

const char* ArcContainer::getComponentType() const {
    return "ArcContainer";
}