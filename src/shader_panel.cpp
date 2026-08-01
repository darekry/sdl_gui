#include "shader_panel.hpp"
#include "gui_manager.hpp"
#include "std.hpp"

ShaderPanel::ShaderPanel(GUIManager& manager, int x, int y, int width, int height)
    : Panel(manager, x, y, width, height) {
}

ShaderPanel::~ShaderPanel() {
    releaseShader();
}

void ShaderPanel::releaseShader() {
    if (m_fragShader) {
        SDL_GPUDevice* device = m_manager.getGPUDevice();
        if (device) {
            SDL_ReleaseGPUShader(device, m_fragShader);
        }
        m_fragShader = nullptr;
    }
}

void ShaderPanel::setShader(const uint8_t* spirvData, size_t spirvSize) {
    releaseShader();
    if (m_renderState) {
        SDL_DestroyGPURenderState(m_renderState);
        m_renderState = nullptr;
    }

    SDL_GPUDevice* device = m_manager.getGPUDevice();
    if (!device) {
        LOG_ERROR("ShaderPanel", "GPU device not available (use SDL_CreateGPURenderer)");
        return;
    }

    Uint64 t0 = SDL_GetTicks();
    SDL_GPUShaderCreateInfo fragInfo{};
    fragInfo.code = spirvData;
    fragInfo.code_size = spirvSize;
    fragInfo.entrypoint = "main";
    fragInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
    fragInfo.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    fragInfo.props = 0;
    m_fragShader = SDL_CreateGPUShader(device, &fragInfo);
    if (!m_fragShader) {
        LOG_ERROR("ShaderPanel", "SDL_CreateGPUShader failed: {}", SDL_GetError());
        return;
    }

    SDL_GPURenderStateCreateInfo stateInfo{};
    stateInfo.fragment_shader = m_fragShader;
    stateInfo.props = 0;
    m_renderState = SDL_CreateGPURenderState(m_manager.getRenderer(), &stateInfo);
    if (!m_renderState) {
        LOG_ERROR("ShaderPanel", "SDL_CreateGPURenderState failed: {}", SDL_GetError());
        releaseShader();
        return;
    }
    LOG_INFO("ShaderPanel", "setShader (GPU shader + render state): {}ms", SDL_GetTicks() - t0);
}

void ShaderPanel::setShaderEnabled(bool enabled) {
    m_shaderEnabled = enabled;
}

void ShaderPanel::setUniformTime(float time) {
    m_uniformTime = time;
}

void ShaderPanel::setUniformMouse(float x, float y) {
    m_uniformMouseX = x;
    m_uniformMouseY = y;
}

bool ShaderPanel::wantsDirectRender() const {
    return true;
}

void ShaderPanel::drawDirect(SDL_Renderer* renderer) {
    if (m_width <= 0 || m_height <= 0) {
        return;
    }

    ensureTempTexture(renderer);
    if (!m_tempTexture) {
        return;
    }

    auto abs_pos = getAbsolutePosition();

    {
        SDL_Texture* oldTarget = SDL_GetRenderTarget(renderer);
        SDL_Rect oldViewport;
        SDL_GetRenderViewport(renderer, &oldViewport);
        SDL_Rect oldClip;
        bool hadClip = SDL_GetRenderClipRect(renderer, &oldClip);
        SDL_SetRenderTarget(renderer, m_tempTexture.get());
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        Panel::draw(renderer);
        SDL_SetRenderTarget(renderer, oldTarget);
        // SDL_SetRenderTarget resetuje viewport/clip — przywróć, by nie przeciekały
        SDL_SetRenderViewport(renderer, &oldViewport);
        SDL_SetRenderClipRect(renderer, hadClip ? &oldClip : nullptr);
    }

    SDL_Vertex verts[4];
    const float x0 = static_cast<float>(abs_pos.x);
    const float y0 = static_cast<float>(abs_pos.y);
    const float x1 = x0 + static_cast<float>(m_width);
    const float y1 = y0 + static_cast<float>(m_height);
    const SDL_FColor c = {m_uniformTime, m_uniformMouseX, m_uniformMouseY, 1.0f};
    verts[0] = {{x0, y0}, c, {0.0f, 0.0f}};
    verts[1] = {{x1, y0}, c, {1.0f, 0.0f}};
    verts[2] = {{x1, y1}, c, {1.0f, 1.0f}};
    verts[3] = {{x0, y1}, c, {0.0f, 1.0f}};
    const int idx[6] = {0, 1, 2, 0, 2, 3};

    if (m_shaderEnabled && m_renderState) {
        SDL_SetGPURenderState(renderer, m_renderState);
    }
    SDL_RenderGeometry(renderer, m_tempTexture.get(), verts, 4, idx, 6);
    if (m_shaderEnabled && m_renderState) {
        SDL_SetGPURenderState(renderer, nullptr);
    }
}

void ShaderPanel::ensureTempTexture(SDL_Renderer* renderer) {
    if (m_tempTexture) {
        float tw = 0.0f, th = 0.0f;
        if (SDL_GetTextureSize(m_tempTexture.get(), &tw, &th) &&
            static_cast<int>(tw) == m_width && static_cast<int>(th) == m_height) {
            return;
        }
        m_tempTexture.reset();
    }
    m_tempTexture.reset(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, m_width, m_height));
    if (m_tempTexture) {
        SDL_SetTextureBlendMode(m_tempTexture.get(), SDL_BLENDMODE_BLEND);
    }
}

const char* ShaderPanel::getComponentType() const {
    return "ShaderPanel";
}
