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

    if (m_shaderEnabled) {
        setGPUState(m_renderState);
    }
}

void ShaderPanel::setShaderEnabled(bool enabled) {
    m_shaderEnabled = enabled;
    setGPUState(enabled ? m_renderState : nullptr);
}

const char* ShaderPanel::getComponentType() const {
    return "ShaderPanel";
}
