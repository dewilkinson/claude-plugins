# {{PROJECT}} -- the executable. A CAULDRON_DX12::FrameworkWindows app ({{PROJECT}}App) over a
# renderer ({{PROJECT}}Renderer); see README.md at the root.
cmake_minimum_required(VERSION 3.24)

project({{PROJECT}}_DX12)

# copyTargetCommand() from the Cauldron framework.
include(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/cauldron/common.cmake)

set(sources
    stdafx.h
    stdafx.cpp
    main.cpp
    {{PROJECT}}App.h
    {{PROJECT}}App.cpp
    {{PROJECT}}Renderer.h
    {{PROJECT}}Renderer.cpp
    dpiawarescaling.manifest   # a .manifest source is merged into the exe's manifest by CMake (MSVC)
)

# Every shader under Shaders/ is copied to bin/ShaderLibDX, where CompileShaderFromFile() looks at
# runtime. CONFIGURE_DEPENDS makes CMake re-glob on each build, so new files are picked up.
file(GLOB shaders CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.hlsl
    ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.hlsli
    ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.h
)
set_source_files_properties(${shaders} PROPERTIES HEADER_FILE_ONLY TRUE)

source_group("Sources" FILES ${sources})
source_group("Shaders" FILES ${shaders})

add_executable({{PROJECT}} WIN32 ${sources} ${shaders})
target_include_directories({{PROJECT}} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Cauldron_DX12 doesn't itself propagate the D3D12/DXGI system libs or the AGS SDK it calls into.
target_link_libraries({{PROJECT}} PRIVATE Cauldron_DX12 ImGUI amd_ags d3d12 dxgi)

if(shaders)
    copyTargetCommand("${shaders}" ${CMAKE_HOME_DIRECTORY}/bin/ShaderLibDX copied_{{PROJECT}}_shaders)
    add_dependencies({{PROJECT}} copied_{{PROJECT}}_shaders)
endif()

set_target_properties({{PROJECT}} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    FOLDER "Apps"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_HOME_DIRECTORY}/bin"
    DEBUG_POSTFIX "d"
)
