// main.cpp
// {{PROJECT}} -- Copyright (c) {{YEAR}} {{AUTHOR}}
//
// Entry point: hands a {{PROJECT}}App to Cauldron's RunFramework, which owns the window, the
// device and the swap chain and calls the app's On*() methods.

#include "stdafx.h"
#include "{{PROJECT}}App.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR lpCmdLine, int nCmdShow)
{
#if defined(_DEBUG)
    {
        Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            debugController->EnableDebugLayer();
    }
#endif

    // COM is needed by the file dialogs / WIC loaders Cauldron and the app may use.
    HRESULT hrCom = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    int result = RunFramework(hInstance, lpCmdLine, nCmdShow, new {{PROJECT}}App("{{WINDOW_TITLE}}"));
    if (SUCCEEDED(hrCom))
        CoUninitialize();
    return result;
}
