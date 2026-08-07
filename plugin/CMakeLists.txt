if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  cmake_minimum_required(VERSION 3.28)
  project(
    bpmdetect-plugin
    LANGUAGES C CXX
    VERSION 0.8.11
    DESCRIPTION "Automatic BPM detection plugin (CLAP, LV2, and VST3)"
    HOMEPAGE_URL "https://github.com/Tatsh/bpmdetect")

  include(FindPkgConfig)

  set(CMAKE_CXX_STANDARD 23)
  set(CMAKE_CXX_EXTENSIONS OFF)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

include(FetchContent)

option(PLUGIN_STATIC_SOUNDTOUCH
       "Statically link SoundTouch into the plugin by building it from source." OFF)
option(USE_SYSTEM_DPF "Use a DPF tree found on the system instead of downloading it." OFF)

if(USE_SYSTEM_DPF)
  # DPF is a source framework, so a usable installation is any tree that carries the CMake glue
  # that provides dpf_add_plugin().
  find_path(
    DPF_SOURCE_DIR
    NAMES cmake/DPF-plugin.cmake
    HINTS "${DPF_ROOT}" "$ENV{DPF_ROOT}"
    PATHS /opt /usr/lib /usr/local/lib /usr/local/share /usr/share
    PATH_SUFFIXES DPF dpf
    DOC "Location of the DPF source tree.")
  if(NOT DPF_SOURCE_DIR)
    message(
      FATAL_ERROR
        "USE_SYSTEM_DPF is ON, but no DPF tree was found. Set DPF_SOURCE_DIR or DPF_ROOT to the \
location of the DPF sources.")
  endif()
  add_subdirectory("${DPF_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/dpf")
else()
  # Pin a known good revision.
  FetchContent_Declare(
    dpf
    GIT_REPOSITORY https://github.com/DISTRHO/DPF.git
    GIT_TAG 4238e1c7f0351bbe488d79f0899c540543ac7583)
  FetchContent_MakeAvailable(dpf)
endif()

if(PLUGIN_STATIC_SOUNDTOUCH)
  # Build SoundTouch from source as a position-independent static library so the plugin bundles
  # do not depend on a system SoundTouch. The pinned commit is the 2.4.1 tag.
  set(BUILD_SHARED_LIBS OFF)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  FetchContent_Declare(
    soundtouch
    GIT_REPOSITORY https://codeberg.org/soundtouch/soundtouch.git
    GIT_TAG b5d0e9ba61871ef135e67bebe3228136da4e8833)
  FetchContent_MakeAvailable(soundtouch)
elseif(NOT TARGET PkgConfig::SOUNDTOUCH)
  pkg_check_modules(SOUNDTOUCH soundtouch REQUIRED IMPORTED_TARGET)
endif()

dpf_add_plugin(
  bpmdetect-plugin
  TARGETS clap lv2 vst3
  FILES_DSP bpmdetectplugin.cpp sampleringbuffer.cpp)
target_include_directories(bpmdetect-plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
if(PLUGIN_STATIC_SOUNDTOUCH)
  target_include_directories(bpmdetect-plugin PUBLIC "${soundtouch_SOURCE_DIR}/include")
  target_link_libraries(bpmdetect-plugin PUBLIC SoundTouch)
else()
  target_link_libraries(bpmdetect-plugin PUBLIC PkgConfig::SOUNDTOUCH)
endif()
