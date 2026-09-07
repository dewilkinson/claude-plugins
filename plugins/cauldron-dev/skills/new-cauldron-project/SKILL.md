---
name: new-cauldron-project
description: Scaffold a brand-new, empty AMD Cauldron (DirectX 12) application in its own directory and generate its Visual Studio solution. Creates the root CMakeLists.txt, CMakeSettings.json, .gitignore, README, a src/DX12 app + renderer skeleton (FrameworkWindows subclass, ImGui, depth buffer, clear + present), the DPI manifest, the Cauldron submodule (cloned from a local checkout when one exists, so it works offline), the prebuilt Cauldron libraries, then runs CMake with the installed Visual Studio generator. Use when the user says "new cauldron project", "empty cauldron app", "scaffold a DX12 sample", "start a new Cauldron sample", "create a cauldron project".
---

# New Cauldron project

Creates an empty, buildable AMD Cauldron DX12 app in a fresh directory, laid out the way SplatLab / Surfels is (root `CMakeLists.txt` + `libs/cauldron` submodule + `src/DX12` executable + `bin/` output). Everything is done by `scaffold.py`, which sits beside this file (in a plugin install: `${CLAUDE_PLUGIN_ROOT}/skills/new-cauldron-project/scaffold.py`; otherwise use Glob for `**/new-cauldron-project/scaffold.py`).

## Inputs

`$ARGUMENTS`: `<ProjectName> [<directory>]`, plus anything the user said in prose.

- **ProjectName** is required and must be a C++ identifier (`HelloCauldron`, `ShadowDemo`). It names the executable, the app class (`<Name>App`) and the renderer class (`<Name>Renderer`). If the user gave none, ask for it with AskUserQuestion; do not invent one.
- **directory** defaults to a sibling of the current repository (`<parent of repo root>/<ProjectName>`, e.g. `C:\github\HelloCauldron`). Use the user's path when given. Refuse to scaffold into a non-empty directory unless the user says to.

## Procedure

1. Run the scaffold (Python 3 is required; `cmake` need not be on PATH, the script finds the Visual Studio copy via vswhere):

   ```
   python <skill dir>/scaffold.py <ProjectName> --dir <directory>
   ```

   Defaults that matter:
   - `--cauldron-from auto` looks for a local Cauldron checkout (`C:\github\splatlab\libs\cauldron`, `C:\github\surfels\libs\cauldron`, or `libs/cauldron` under the current repo) and clones it locally at the same commit, pointing `origin` at `https://github.com/dewilkinson/Cauldron.git` (the fork with the prebuilt-lib and DXC patches). With no local checkout it does a network `git submodule add` from that URL. Pass a path or `--cauldron-url` to override.
   - The prebuilt `Cauldron_Common` / `Cauldron_DX12` / `ImGUI` static libraries next to that checkout (`libs/cauldron-prebuilt`) are copied so the first build takes seconds instead of compiling the framework. `--no-prebuilt` skips this. When Git LFS is installed they are tracked through it, as in SplatLab.
   - The project is `git init`-ed and committed. `--no-commit` leaves it staged. `--no-configure` skips CMake. `--build Debug|Release` builds after configuring.
   - `--generator` overrides the Visual Studio generator (auto-detected from the newest installed VS; VS 2026 emits a `.slnx`, older ones a `.sln`).

2. Read the script's final summary and relay it: project path, solution file, and the executable that lands in `bin/`.

3. If the user asked for it to be built or run, run `cmake --build build --config Debug` in the project directory (or pass `--build Debug`) and launch `bin/<Name>d.exe`. The window shows a clear colour and a small ImGui panel; that is the whole app.

## What the generated project contains

| Path | Purpose |
|---|---|
| `CMakeLists.txt` | Forces `GFX_API=DX12` (no Vulkan SDK needed), MSVC 142+ check, `bin/` output dir, `/MP`, Release builds with PDBs, `CAULDRON_USE_PREBUILT` option with from-source fallback, startup project, Cauldron targets swept into a `ThirdParty/Cauldron` solution folder. |
| `CMakeSettings.json` | So "Open Folder" in Visual Studio uses the VS generator (Ninja breaks on Cauldron's duplicate shader copies). |
| `src/DX12/CMakeLists.txt` | The `WIN32` executable: links `Cauldron_DX12 ImGUI amd_ags d3d12 dxgi`, embeds the DPI manifest, copies every `.hlsl`/`.h` under `Shaders/` to `bin/ShaderLibDX` (where `CompileShaderFromFile` looks at runtime), debug binaries get a `d` suffix, debugger working dir is `bin/`. |
| `src/DX12/main.cpp` | `WinMain`: enables the D3D12 debug layer in Debug, COM init, `RunFramework(new <Name>App)`. |
| `src/DX12/<Name>App.h/.cpp` | `FrameworkWindows` subclass: `OnCreate` (InitDirectXCompiler, shader cache, renderer, ImGui), `OnRender` (BeginFrame, ImGui frame, renderer, EndFrame), `OnEvent` (ImGui input), `OnResize`, `OnUpdateDisplay`, `OnDestroy`. |
| `src/DX12/<Name>Renderer.h/.cpp` | Descriptor heaps, upload heap, constant-buffer ring, command-list ring, Cauldron ImGUI, depth buffer; per frame: wait, barrier, clear, ImGui draw, present barrier, submit. Add passes in `OnRender`. |
| `src/DX12/stdafx.h` | Windows + DirectXMath + the Cauldron base headers, `NOMINMAX` and the `min`/`max` un-define the vendored headers need. |
| `src/DX12/Shaders/` | Empty; drop `.hlsl` files here, re-run CMake (the glob is `CONFIGURE_DEPENDS`). |
| `libs/cauldron` | Submodule. `libs/cauldron-prebuilt/lib/{Debug,Release}` when copied. |

## Gotchas the template already handles (keep them when editing generated code)

- `InitDirectXCompiler()` must run before `CreateShaderCache()` and any shader compile, or every compile fails silently.
- Use a Visual Studio generator, never Ninja, for anything that includes Cauldron.
- The prebuilt `.lib` files only match the Cauldron commit they were built from; when bumping the submodule, rebuild with `-DCAULDRON_USE_PREBUILT=OFF` or refresh the libs.
- The vendored ImGui is 1.53 with a static default context: no `ImGui::CreateContext()` call is needed or possible.
