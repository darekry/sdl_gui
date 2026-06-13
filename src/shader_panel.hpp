#pragma once
#include "panel.hpp"
#include <SDL3/SDL_gpu.h>

class ShaderPanel : public Panel {
public:
    ShaderPanel(GUIManager& manager, int x, int y, int width, int height);
    ~ShaderPanel() override;

    void setShader(const uint8_t* spirvData, size_t spirvSize);
    void setShaderEnabled(bool enabled);
    [[nodiscard]] bool isShaderEnabled() const { return m_shaderEnabled; }

    const char* getComponentType() const override;

private:
    void releaseShader();

    SDL_GPUShader* m_fragShader = nullptr;
    SDL_GPURenderState* m_renderState = nullptr;
    bool m_shaderEnabled = true;
};
