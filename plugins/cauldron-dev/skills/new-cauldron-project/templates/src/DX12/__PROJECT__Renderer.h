// {{PROJECT}}Renderer.h
// {{PROJECT}} -- Copyright (c) {{YEAR}} {{AUTHOR}}
//
// Everything that touches the GPU: the descriptor heaps, upload heap, per-frame rings, the depth
// buffer and the frame itself. The app hands it a State each frame; add your own passes to
// OnRender() and their resources to OnCreate() / OnCreateWindowSizeDependentResources().

#pragma once
#include "stdafx.h"

class {{PROJECT}}Renderer
{
public:
    // What the app tells the renderer each frame. Grow this as the app grows.
    struct State
    {
        float clearColor[4] = { 0.10f, 0.11f, 0.13f, 1.0f };
        float deltaTime = 0.0f; // milliseconds
    };

    void OnCreate(Device* pDevice, SwapChain* pSwapChain);
    void OnDestroy();

    void OnCreateWindowSizeDependentResources(SwapChain* pSwapChain, uint32_t width, uint32_t height);
    void OnDestroyWindowSizeDependentResources();
    void OnUpdateDisplayDependentResources(SwapChain* pSwapChain);

    void OnRender(const State* pState, SwapChain* pSwapChain);

private:
    // Frames that can be in flight; the rings are sized for this many. Must be >= the swap
    // chain's back-buffer count (Cauldron creates it with 2).
    static constexpr uint32_t kBackBufferCount = 3;

    Device*  m_pDevice = nullptr;
    uint32_t m_width = 0;
    uint32_t m_height = 0;

    ResourceViewHeaps m_resourceViewHeaps;
    UploadHeap        m_uploadHeap;
    DynamicBufferRing m_constantBufferRing;
    CommandListRing   m_commandListRing;
    ImGUI             m_imGui;

    Texture m_depthBuffer;
    DSV     m_depthBufferDSV;
};
