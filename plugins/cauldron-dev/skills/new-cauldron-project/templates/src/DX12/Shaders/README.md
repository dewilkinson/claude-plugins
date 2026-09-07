Put `.hlsl` / `.hlsli` / `.h` shader files here. `src/DX12/CMakeLists.txt` globs this directory and
copies every file to `bin/ShaderLibDX/` at build time, which is where Cauldron's `CompileShaderFromFile()`
looks. Re-run CMake (or just build; the glob is `CONFIGURE_DEPENDS`) after adding a file.
