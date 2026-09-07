// {{PROJECT}}App.cpp
// {{PROJECT}} -- Copyright (c) {{YEAR}} {{AUTHOR}}

#include "stdafx.h"
#include "{{PROJECT}}App.h"

{{PROJECT}}App::{{PROJECT}}App(LPCSTR name)
    : FrameworkWindows(name)
{
}

// Runs before the window exists: pick the initial window size and read the command line.
void {{PROJECT}}App::OnParseCommandLine(LPSTR lpCmdLine, uint32_t* pWidth, uint32_t* pHeight)
{
    *pWidth = 1280;
    *pHeight = 720;
    (void)lpCmdLine; // parse app options here
}

// The device and swap chain already exist (FrameworkWindows::DeviceInit ran first).
void {{PROJECT}}App::OnCreate()
{
    // Both must precede any shader compile: without InitDirectXCompiler() every
    // CompileShaderFromFile() fails silently.
    InitDirectXCompiler();
    CreateShaderCache();

    m_pRenderer = new {{PROJECT}}Renderer();
    m_pRenderer->OnCreate(&m_device, &m_swapChain);

    ImGUI_Init((void*)m_windowHwnd);

    // Size-dependent resources for the initial window size; later resizes come through OnResize().
    OnResize(true);
    OnUpdateDisplay();

    m_swapChain.SetVSync(m_vsync);
}

void {{PROJECT}}App::OnDestroy()
{
    ImGUI_Shutdown();

    m_device.GPUFlush();

    m_pRenderer->OnDestroyWindowSizeDependentResources();
    m_pRenderer->OnDestroy();
    delete m_pRenderer;
    m_pRenderer = nullptr;

    DestroyShaderCache(&m_device);
}

// Raw window messages; return true when consumed.
bool {{PROJECT}}App::OnEvent(MSG msg)
{
    if (ImGUI_WndProcHandler(msg.hwnd, msg.message, msg.wParam, msg.lParam))
        return true;
    return false;
}

// Called by Cauldron when the window size changes (and once from OnCreate above).
void {{PROJECT}}App::OnResize(bool resizeRender)
{
    if (resizeRender && m_Width && m_Height && m_pRenderer)
    {
        m_pRenderer->OnDestroyWindowSizeDependentResources();
        m_pRenderer->OnCreateWindowSizeDependentResources(&m_swapChain, m_Width, m_Height);
    }
}

// Display mode / swap-chain format changes (e.g. HDR toggles).
void {{PROJECT}}App::OnUpdateDisplay()
{
    if (m_pRenderer)
        m_pRenderer->OnUpdateDisplayDependentResources(&m_swapChain);
}

void {{PROJECT}}App::BuildUI()
{
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(320.0f, 0.0f), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("{{WINDOW_TITLE}}"))
    {
        ImGui::Text("%s", m_systemInfo.mGPUName.c_str());
        ImGui::Text("%.2f ms / frame (%.0f fps)", m_deltaTime, m_deltaTime > 0.0 ? 1000.0 / m_deltaTime : 0.0);
        ImGui::Separator();
        ImGui::ColorEdit3("Clear colour", m_state.clearColor);
        if (ImGui::Checkbox("VSync", &m_vsync))
            m_swapChain.SetVSync(m_vsync);
    }
    ImGui::End();
}

void {{PROJECT}}App::OnRender()
{
    BeginFrame(); // updates m_deltaTime

    ImGUI_UpdateIO(m_Width, m_Height);
    ImGui::NewFrame();
    BuildUI();

    m_state.deltaTime = (float)m_deltaTime;
    m_pRenderer->OnRender(&m_state, &m_swapChain);

    EndFrame(); // presents
}
