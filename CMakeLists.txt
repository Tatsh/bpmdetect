cmake_minimum_required(VERSION 3.31)
project(
  bpmdetect
  LANGUAGES C CXX
  VERSION 0.8.8
  DESCRIPTION "Automatic BPM detection tool"
  HOMEPAGE_URL "https://github.com/Tatsh/bpmdetect")

include(FeatureSummary)
include(FindPkgConfig)
include(GNUInstallDirs)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BPMDETECT_CXX_WARN_ALL "Do not use." OFF)
mark_as_advanced(BPMDETECT_CXX_WARN_ALL)
option(BUILD_DOCS "Build documentation." OFF)
option(BUILD_DOCS_ONLY "Build documentation only." OFF)
option(BUILD_TESTS "Build tests." OFF)
option(COVERAGE "Enable code coverage." OFF)
option(NON_PORTABLE_MACOS_BUNDLE
       "Create a macOS bundle which will depend on libraries within FHS-compliant directories \
(Homebrew, MacPorts). Implies FHS=ON." OFF)
option(NO_GUI "Build CLI-only version.")
set(BUNDLE_INSTALL_PREFIX
    .
    CACHE FILEPATH "Install the bundle to a custom directory. This can be an absolute path or \
relative to CMAKE_INSTALL_PREFIX.")
option(I18N "Enable i18n support." OFF)
option(ENABLE_DESKTOP_PORTAL "Build with support for desktop portal (Flatpak)." OFF)

if(NOT APPLE)
  set(NON_PORTABLE_MACOS_BUNDLE
      OFF
      CACHE BOOL "Non-portable macOS bundle mode." FORCE)
endif()
if(APPLE)
  if(NON_PORTABLE_MACOS_BUNDLE)
    set(FHS
        ON
        CACHE BOOL "Use FHS directories." FORCE)
  else()
    set(EXTRA_DOCS
        ON
        CACHE BOOL "Install extra documentation.")
  endif()
endif()
if(CMAKE_SYSTEM_NAME MATCHES "Linux|MSYS|CYGWIN|FreeBSD|NetBSD|OpenBSD|DragonFly")
  set(FHS
      ON
      CACHE BOOL "Use FHS directories.")
endif()

if(NOT BUILD_DOCS_ONLY)
  find_package(ECM NO_MODULE REQUIRED)
  list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
  include(ECMQtDeclareLoggingCategory)

  find_package(Qt6Core 6.7 CONFIG REQUIRED)
  if(NOT NO_GUI)
    find_package(Qt6Gui 6.7 CONFIG REQUIRED)
    find_package(Qt6LinguistTools 6.7 CONFIG REQUIRED)
    find_package(Qt6Multimedia 6.7 CONFIG REQUIRED)
    find_package(Qt6Widgets 6.7 CONFIG REQUIRED)
  endif()
  qt_standard_project_setup()
  pkg_check_modules(
    FFMPEG
    libavformat
    libavcodec
    libavutil
    REQUIRED
    IMPORTED_TARGET
    GLOBAL)
  pkg_check_modules(SOUNDTOUCH soundtouch REQUIRED IMPORTED_TARGET GLOBAL)

  feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

  # Sane flags from Kate
  add_compile_definitions(
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_FROM_BYTEARRAY
    QT_NO_CAST_TO_ASCII
    QT_NO_SIGNALS_SLOTS_KEYWORDS
    QT_NO_URL_CAST_FROM_STRING
    QT_STRICT_ITERATORS
    QT_USE_FAST_OPERATOR_PLUS
    QT_USE_QSTRINGBUILDER
    $<$<BOOL:${NO_GUI}>:NO_GUI>)
  add_compile_options($<$<AND:$<CXX_COMPILER_ID:Clang,GNU>,$<BOOL:${COVERAGE}>>:--coverage>)
  add_link_options($<$<AND:$<CXX_COMPILER_ID:Clang,GNU>,$<BOOL:${COVERAGE}>>:--coverage>)

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND BPMDETECT_CXX_WARN_ALL)
      add_compile_options(
        -fsafe-buffer-usage-suggestions
        -Weverything
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-exit-time-destructors
        -Wno-global-constructors
        -Wno-padded)
    endif()
  endif()

  add_subdirectory(src)
  if(BUILD_TESTS)
    find_package(Qt6Test CONFIG REQUIRED)
    enable_testing()
    add_subdirectory(tests)
  endif()
endif()

if(BUILD_DOCS OR BUILD_DOCS_ONLY)
  set(DOXYGEN_GENERATE_TAGFILE "${CMAKE_BINARY_DIR}/html/bpmdetect.tags")
  set(DOXYGEN_INTERACTIVE_SVG YES)
  set(DOXYGEN_MARKDOWN_ID_STYLE GITHUB)
  set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md")
  set(DOXYGEN_TAGFILES
      ./.docs/cppreference-doxygen-web.tag.xml=https://en.cppreference.com/w/
      ./.docs/qtcore.tags=http://doc.qt.io/qt-6/
      ./.docs/qtgui.tags=http://doc.qt.io/qt-6/
      ./.docs/qtwidgets.tags=http://doc.qt.io/qt-6/
      ./.docs/soundtouch.tags=https://codeberg.org/soundtouch/soundtouch/)
  find_package(Doxygen REQUIRED COMPONENTS dot)
  doxygen_add_docs(docs README.md src ALL)
endif()
