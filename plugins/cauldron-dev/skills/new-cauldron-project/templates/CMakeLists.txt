cmake_minimum_required(VERSION 3.24)

# Build Cauldron (the vendored framework under libs/cauldron) DX12-only so the
# Vulkan SDK is never required just to build this project.
set(GFX_API "DX12")

project({{PROJECT}})

if(MSVC_TOOLSET_VERSION VERSION_LESS 142)
    message(FATAL_ERROR "Cannot find MSVC toolset version 142 or greater. Please make sure Visual Studio 2019 or newer is installed")
endif()

# Executables (and the shaders / runtime DLLs Cauldron copies next to them) go to bin/.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_HOME_DIRECTORY}/bin)
endforeach()

add_compile_options(/MP)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Prebuilt Cauldron: when libs/cauldron-prebuilt/lib/{Debug,Release}/*.lib exist, the three Cauldron
# CMakeLists.txt that build Cauldron_Common / Cauldron_DX12 / ImGUI create IMPORTED targets pointing at
# them instead of compiling ~50 framework sources (this needs the dewilkinson/Cauldron fork, which is
# what the submodule points at). Falls back to a from-source build automatically when the .lib files
# are missing (not copied, or Git LFS objects not pulled). Pass -DCAULDRON_USE_PREBUILT=OFF to force a
# from-source build, e.g. when patching Cauldron.
option(CAULDRON_USE_PREBUILT "Link prebuilt Cauldron static libraries instead of building from source" ON)
if(CAULDRON_USE_PREBUILT AND NOT EXISTS "${CMAKE_HOME_DIRECTORY}/libs/cauldron-prebuilt/lib/Release/Cauldron_DX12.lib")
    message(STATUS "libs/cauldron-prebuilt/lib/Release/Cauldron_DX12.lib is missing -- building Cauldron from source instead.")
    set(CAULDRON_USE_PREBUILT OFF)
endif()

# Release keeps its full optimizer settings but also emits a PDB so a debugger can resolve symbols.
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")

add_subdirectory(libs/cauldron)
add_subdirectory(src/DX12)

# Otherwise Visual Studio picks ALL_BUILD as the startup project and F5 fails.
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT {{PROJECT}})

# Cauldron's own CMakeLists.txt files don't set FOLDER, so its targets would land at the solution
# root next to ours. Sweep everything defined under libs/cauldron into one Solution Explorer folder.
function(group_targets_under_folder dir folder)
    get_property(subdirs DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
    foreach(subdir ${subdirs})
        group_targets_under_folder(${subdir} ${folder})
    endforeach()
    get_property(targets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
    foreach(tgt ${targets})
        get_target_property(isImported ${tgt} IMPORTED)
        if(NOT isImported)
            set_target_properties(${tgt} PROPERTIES FOLDER ${folder})
        endif()
    endforeach()
endfunction()
group_targets_under_folder(${CMAKE_SOURCE_DIR}/libs/cauldron "ThirdParty/Cauldron")
