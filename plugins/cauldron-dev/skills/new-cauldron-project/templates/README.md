# {{PROJECT}}

An empty [AMD Cauldron](https://github.com/GPUOpen-LibrariesAndSDKs/Cauldron) DirectX 12 application: a window, a cleared back buffer, a depth buffer and an ImGui panel. Add render passes in `src/DX12/{{PROJECT}}Renderer.cpp`, UI in `{{PROJECT}}App::BuildUI()`, and shaders under `src/DX12/Shaders/` (copied to `bin/ShaderLibDX` at build time for `CompileShaderFromFile`).

## Building

CMake 3.24+ and Visual Studio 2019 or newer (MSVC toolset 142+) with the Windows 10 SDK. No Vulkan SDK; the root `CMakeLists.txt` forces `GFX_API=DX12`.

```bat
git clone --recurse-submodules <this repo>
cd {{PROJECT}}
cmake -S . -B build -G "{{GENERATOR}}" -A x64
cmake --build build --config Debug
bin\{{PROJECT}}d.exe
```

Use whichever Visual Studio generator matches your install (`"Visual Studio 17 2022"` works the same way), and open the solution in `build/` to work in the IDE. **Use a Visual Studio generator, not Ninja**: Cauldron's `src/Common` and `src/DX12` both copy overlapping FidelityFX headers into `bin/ShaderLibDX`, which Ninja rejects.

`libs/cauldron` is the [dewilkinson/Cauldron]({{CAULDRON_URL}}) fork (prebuilt static-lib support, DXC / Agility SDK path fixes, device-lost hardening). When `libs/cauldron-prebuilt/lib/{Debug,Release}/` holds `Cauldron_Common`, `Cauldron_DX12` and `ImGUI` (tracked through Git LFS: `git lfs install` once per machine, or `git lfs pull` after cloning), the `CAULDRON_USE_PREBUILT` option (default on) links them instead of compiling the framework; it falls back to a from-source build when they are missing, and `-DCAULDRON_USE_PREBUILT=OFF` forces that. The prebuilt libraries only match the submodule commit they were built from.

## Layout

| Path | Contents |
|---|---|
| `src/DX12/main.cpp` | `WinMain`: D3D12 debug layer in Debug builds, COM, `RunFramework`. |
| `src/DX12/{{PROJECT}}App.*` | `CAULDRON_DX12::FrameworkWindows` subclass; owns the renderer and the ImGui frame. |
| `src/DX12/{{PROJECT}}Renderer.*` | Descriptor heaps, upload heap, constant-buffer and command-list rings, depth buffer, the frame. |
| `src/DX12/stdafx.h` | Windows, DirectXMath and the Cauldron base headers. |
| `src/DX12/Shaders/` | HLSL, copied to `bin/ShaderLibDX/`. |
| `bin/` | Build output: executables, `ShaderLibDX/`, Cauldron's runtime DLLs. |
