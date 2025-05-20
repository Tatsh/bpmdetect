cmake_minimum_required(VERSION 3.10)
project(bpmdetect)

include(FindPkgConfig)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(
  Qt6
  COMPONENTS Core Gui Multimedia Widgets
  REQUIRED)
pkg_check_modules(FLAC flac)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)
pkg_check_modules(MAD mad)
pkg_check_modules(TAGLIB taglib)
pkg_check_modules(VORBIS vorbisfile)

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

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
  if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND DEVMGMT_CXX_WARN_ERROR_ALL)
    add_compile_options(
      -Weverything
      -Wno-c++98-compat
      -Wno-c++98-compat-pedantic
      -Wno-deprecated-copy-with-user-provided-dtor
      -Wno-exit-time-destructors
      -Wno-global-constructors
      -Wno-inconsistent-missing-destructor-override
      -Wno-padded
      -Wno-redundant-parens)
  endif ()
endif ()

add_subdirectory(src)
