// {{PROJECT}}App.h
// {{PROJECT}} -- Copyright (c) {{YEAR}} {{AUTHOR}}
//
// The application: a CAULDRON_DX12::FrameworkWindows subclass. Cauldron owns the window, the
// D3D12 device (m_device) and the swap chain (m_swapChain) and calls the On*() methods below;
// this class owns the renderer and builds the ImGui frame.

#pragma once
#include "stdafx.h"
#include "{{PROJECT}}Renderer.h"

class {{PROJECT}}App : public CAULDRON_DX12::FrameworkWindows
{
public:
    explicit {{PROJECT}}App(LPCSTR name);

    // FrameworkWindows interface, called from Cauldron's RunFramework message loop.
    void OnParseCommandLine(LPSTR lpCmdLine, uint32_t* pWidth, uint32_t* pHeight) override;
    void OnCreate() override;
    void OnDestroy() override;
    void OnRender() override;
    bool OnEvent(MSG msg) override;
    void OnResize(bool resizeRender) override;
    void OnUpdateDisplay() override;

private:
    void BuildUI();

    {{PROJECT}}Renderer*       m_pRenderer = nullptr;
    {{PROJECT}}Renderer::State m_state;
    bool                       m_vsync = true;
};
