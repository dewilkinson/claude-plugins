// {{PROJECT}}Renderer.cpp
// {{PROJECT}} -- Copyright (c) {{YEAR}} {{AUTHOR}}

#include "stdafx.h"
#include "{{PROJECT}}Renderer.h"

void {{PROJECT}}Renderer::OnCreate(Device* pDevice, SwapChain* pSwapChain)
{
    m_pDevice = pDevice;

    // Descriptor heaps: CBV, SRV, UAV, DSV, RTV, sampler counts. Raise them as passes are added.
    m_resourceViewHeaps.OnCreate(pDevice, 256, 256, 256, 64, 64, 64);
    m_resourceViewHeaps.AllocDSVDescriptor(1, &m_depthBufferDSV);

    // Staging memory for one-off uploads (textures, static buffers, the ImGui font atlas).
    m_uploadHeap.OnCreate(pDevice, 32 * 1024 * 1024);

    // Per-frame constant buffers, one region per in-flight frame.
    m_constantBufferRing.OnCreate(pDevice, kBackBufferCount, 8 * 1024 * 1024, &m_resourceViewHeaps);

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    m_commandListRing.OnCreate(pDevice, kBackBufferCount, 2, queueDesc);

    // Cauldron's ImGui backend: pipeline for the swap chain's format, font atlas via the upload heap.
    m_imGui.OnCreate(pDevice, &m_uploadHeap, &m_resourceViewHeaps, &m_constantBufferRing, pSwapChain->GetFormat());

    // Flush the font atlas (and anything else queued above) to the GPU before the first frame.
    m_uploadHeap.FlushAndFinish();
}

void {{PROJECT}}Renderer::OnDestroy()
{
    m_imGui.OnDestroy();
    m_commandListRing.OnDestroy();
    m_constantBufferRing.OnDestroy();
    m_uploadHeap.OnDestroy();
    m_resourceViewHeaps.OnDestroy();
}

void {{PROJECT}}Renderer::OnCreateWindowSizeDependentResources(SwapChain* /*pSwapChain*/, uint32_t width, uint32_t height)
{
    m_width = width;
    m_height = height;

    m_depthBuffer.InitDepthStencil(m_pDevice, "{{PROJECT}}Renderer::m_depthBuffer",
        &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL), 1.0f);
    m_depthBuffer.CreateDSV(0, &m_depthBufferDSV);
}

void {{PROJECT}}Renderer::OnDestroyWindowSizeDependentResources()
{
    m_depthBuffer.OnDestroy();
}

void {{PROJECT}}Renderer::OnUpdateDisplayDependentResources(SwapChain* pSwapChain)
{
    // The swap chain format may have changed (SDR / HDR); rebuild the pipelines that render into it.
    m_imGui.UpdatePipeline(pSwapChain->GetFormat());
}

void {{PROJECT}}Renderer::OnRender(const State* pState, SwapChain* pSwapChain)
{
    // Throttle the CPU so it never overwrites a command allocator / ring region the GPU still reads.
    pSwapChain->WaitForSwapChain();

    m_constantBufferRing.OnBeginFrame();
    m_commandListRing.OnBeginFrame();

    ID3D12GraphicsCommandList2* pCmdLst = m_commandListRing.GetNewCommandList();

    ID3D12DescriptorHeap* descriptorHeaps[] = { m_resourceViewHeaps.GetCBV_SRV_UAVHeap() };
    pCmdLst->SetDescriptorHeaps(1, descriptorHeaps);

    ID3D12Resource* pBackBuffer = pSwapChain->GetCurrentBackBufferResource();
    D3D12_RESOURCE_BARRIER toRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(pBackBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    pCmdLst->ResourceBarrier(1, &toRenderTarget);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = *pSwapChain->GetCurrentBackBufferRTV();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_depthBufferDSV.GetCPU();
    pCmdLst->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
    pCmdLst->ClearRenderTargetView(rtvHandle, pState->clearColor, 0, nullptr);
    pCmdLst->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    CD3DX12_VIEWPORT viewport(0.0f, 0.0f, (float)m_width, (float)m_height);
    CD3DX12_RECT scissor(0, 0, (LONG)m_width, (LONG)m_height);
    pCmdLst->RSSetViewports(1, &viewport);
    pCmdLst->RSSetScissorRects(1, &scissor);

    // ---- Scene passes go here ----------------------------------------------------------------

    // UI last, on top of everything.
    m_imGui.Draw(pCmdLst);

    D3D12_RESOURCE_BARRIER toPresent = CD3DX12_RESOURCE_BARRIER::Transition(pBackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    pCmdLst->ResourceBarrier(1, &toPresent);

    ThrowIfFailed(pCmdLst->Close());
    ID3D12CommandList* pCmdLists[] = { pCmdLst };
    m_pDevice->GetGraphicsQueue()->ExecuteCommandLists(1, pCmdLists);
}
