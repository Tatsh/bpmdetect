cmake_minimum_required(VERSION 3.31)
project(bpmdetect VERSION 0.7.2)

find_package(ECM NO_MODULE REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(ECMQtDeclareLoggingCategory)
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
option(BUILD_TESTS "Build tests." OFF)
option(COVERAGE "Enable code coverage." OFF)
option(ENABLE_FLAC "Enable FLAC support." OFF)
option(ENABLE_MP3 "Enable MP3 support." OFF)
option(ENABLE_VORBIS "Enable Vorbis support." OFF)
option(ENABLE_WAVPACK "Enable WavPack support." OFF)
option(I18N "Enable i18n support." OFF)

find_package(Qt6Core CONFIG REQUIRED)
find_package(Qt6Gui CONFIG REQUIRED)
find_package(Qt6LinguistTools CONFIG REQUIRED)
find_package(Qt6Multimedia CONFIG REQUIRED)
find_package(Qt6Widgets CONFIG REQUIRED)
pkg_check_modules(SOUNDTOUCH soundtouch REQUIRED IMPORTED_TARGET GLOBAL)
pkg_check_modules(TAGLIB taglib REQUIRED IMPORTED_TARGET GLOBAL)

if(ENABLE_FLAC)
  pkg_check_modules(FLAC flac REQUIRED IMPORTED_TARGET GLOBAL)
endif()
if(ENABLE_MP3)
  pkg_check_modules(MAD mad REQUIRED IMPORTED_TARGET GLOBAL)
endif()
if(ENABLE_VORBIS)
  pkg_check_modules(VORBIS vorbisfile REQUIRED IMPORTED_TARGET GLOBAL)
endif()
if(ENABLE_WAVPACK)
  pkg_check_modules(WAVPACK wavpack REQUIRED IMPORTED_TARGET GLOBAL)
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

# Sane flags from Kate
add_definitions(
  -DQT_NO_CAST_FROM_ASCII
  -DQT_NO_CAST_FROM_BYTEARRAY
  -DQT_NO_CAST_TO_ASCII
  -DQT_NO_SIGNALS_SLOTS_KEYWORDS
  -DQT_NO_URL_CAST_FROM_STRING
  -DQT_STRICT_ITERATORS
  -DQT_USE_FAST_OPERATOR_PLUS
  -DQT_USE_QSTRINGBUILDER)
add_compile_options($<$<AND:$<CXX_COMPILER_ID:Clang,GNU>,$<BOOL:${COVERAGE}>>:--coverage>)
add_link_options($<$<AND:$<CXX_COMPILER_ID:Clang,GNU>,$<BOOL:${COVERAGE}>>:--coverage>)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND BPMDETECT_CXX_WARN_ALL)
    add_compile_options(
      -fsafe-buffer-usage-suggestions
      -Weverything
      -Wno-c++98-compat
      -Wno-c++98-compat-pedantic
      -Wno-missing-prototypes
      -Wno-missing-variable-declarations
      -Wno-unsafe-buffer-usage)
  endif()
endif()

add_subdirectory(src)
if(BUILD_TESTS)
  find_package(Qt6Test CONFIG REQUIRED)
  enable_testing()
  add_subdirectory(tests)
endif()
