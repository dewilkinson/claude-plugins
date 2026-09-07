// stdafx.h
// {{PROJECT}} -- Copyright (c) {{YEAR}} {{AUTHOR}}
//
// Shared header for every source file in the app: Windows, DirectXMath, and the Cauldron
// framework base headers.
#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX // otherwise windows.h's max()/min() macros break std::max/std::min call sites
#include <windows.h>
#include <windowsx.h>
#include <wrl/client.h>

#include <cstdint>
#include <algorithm>
#include <string>
#include <vector>
#include <chrono>

#include <DirectXMath.h>
using namespace DirectX;

// Cauldron (libs/cauldron/src/DX12 and libs/cauldron/src/common, both include dirs of the
// Cauldron_DX12 CMake target).
#include "base/Device.h"
#include "base/SwapChain.h"
#include "base/FrameworkWindows.h"
#include "base/ResourceViewHeaps.h"
#include "base/UploadHeap.h"
#include "base/DynamicBufferRing.h"
#include "base/CommandListRing.h"
#include "base/StaticBufferPool.h"
#include "base/Texture.h"
#include "base/ShaderCompilerHelper.h"
#include "base/DXCHelper.h"     // InitDirectXCompiler() -- must be called before any shader compile
#include "base/Helper.h"
#include "base/GPUTimestamps.h"
#include "base/Imgui.h"
#include "base/ImGuiHelper.h"

#include "Misc/Misc.h"
#include "Misc/Error.h"    // ThrowIfFailed()

// Something transitively included still leaves the windows.h min()/max() macros active despite
// NOMINMAX above (observed with the vendored Cauldron/AGS headers); undef them defensively.
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

using namespace CAULDRON_DX12;
