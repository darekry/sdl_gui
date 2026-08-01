#pragma once
#include "std.hpp"
#include "panel.hpp"
#include <SDL3/SDL_gpu.h>

class ShaderPanel : public Panel {
public:
    ShaderPanel(GUIManager& manager, int x, int y, int width, int height);
    ~ShaderPanel() override;

    void setShader(const uint8_t* spirvData, size_t spirvSize);
    void setShaderEnabled(bool enabled);
    [[nodiscard]] bool isShaderEnabled() const { return m_shaderEnabled; }

    void setUniformTime(float time);
    void setUniformMouse(float x, float y);

    bool wantsDirectRender() const override;
    void drawDirect(SDL_Renderer* renderer) override;

    const char* getComponentType() const override;

private:
    void releaseShader();
    void ensureTempTexture(SDL_Renderer* renderer);

    SDL_GPUShader* m_fragShader = nullptr;
    SDL_GPURenderState* m_renderState = nullptr;
    bool m_shaderEnabled = true;
    std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> m_tempTexture{nullptr, SDL_DestroyTexture};
    float m_uniformTime = 0.0f;
    float m_uniformMouseX = -1.0f;
    float m_uniformMouseY = -1.0f;
};
