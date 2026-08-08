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
  # do not depend on a system SoundTouch. The archive of the 2.4.1 tag is pinned by content hash;
  # a tarball download is far less prone to transient failures than a git clone.
  set(BUILD_SHARED_LIBS OFF)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  set(SOUNDSTRETCH
      OFF
      CACHE BOOL "Build soundstretch command line utility." FORCE)
  FetchContent_Declare(
    soundtouch
    URL https://codeberg.org/soundtouch/soundtouch/archive/2.4.1.tar.gz
    URL_HASH SHA256=35d404e6e8c2ebd12fb4000da6fadd75c99e37eed2126a04721828c11c0377ec)
  FetchContent_MakeAvailable(soundtouch)
elseif(NOT TARGET PkgConfig::SOUNDTOUCH)
  pkg_check_modules(SOUNDTOUCH soundtouch REQUIRED IMPORTED_TARGET)
endif()

dpf_add_plugin(
  bpmdetect-plugin
  TARGETS clap lv2 vst3
  FILES_DSP bpmdetectplugin.cpp sampleringbuffer.cpp
  FILES_UI bpmdetectui.cpp)

# DPF's CMake glue force-enables its web view support, which does not compile without GNU
# extensions and whose Windows implementation does not compile with Clang on ARM64. The UI does
# not use a web view, so strip the definition and the web view sources.
foreach(property COMPILE_DEFINITIONS INTERFACE_COMPILE_DEFINITIONS)
  get_target_property(dglDefinitions dgl-opengl "${property}")
  if(dglDefinitions)
    list(REMOVE_ITEM dglDefinitions DGL_USE_WEB_VIEW)
    set_target_properties(dgl-opengl PROPERTIES "${property}" "${dglDefinitions}")
  endif()
endforeach()
get_target_property(dglSources dgl-opengl SOURCES)
list(FILTER dglSources EXCLUDE REGEX "WebView")
set_target_properties(dgl-opengl PROPERTIES SOURCES "${dglSources}")

# DPF links OpenGL through a find module variable that is empty when another OpenGL discovery ran
# first, which the application's Qt configuration does. Link the imported target explicitly.
if(UNIX AND NOT APPLE)
  find_package(OpenGL REQUIRED)
  target_link_libraries(bpmdetect-plugin PUBLIC OpenGL::GL)
endif()
target_include_directories(bpmdetect-plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
if(PLUGIN_STATIC_SOUNDTOUCH)
  target_include_directories(bpmdetect-plugin PUBLIC "${soundtouch_SOURCE_DIR}/include")
  target_link_libraries(bpmdetect-plugin PUBLIC SoundTouch)
else()
  target_link_libraries(bpmdetect-plugin PUBLIC PkgConfig::SOUNDTOUCH)
endif()
