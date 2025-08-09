cmake_minimum_required(VERSION 3.31)
project(bpmdetect VERSION 0.7.1)

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

option(BPMDETECT_CXX_WARN_ERROR_ALL "Do not use." OFF)
option(BUILD_TESTS "Build tests." OFF)
option(COVERAGE "Enable code coverage." OFF)
option(I18N "Enable i18n support" OFF)
mark_as_advanced(BPMDETECT_CXX_WARN_ERROR_ALL)

find_package(Qt6Core REQUIRED)
find_package(Qt6Gui REQUIRED)
find_package(Qt6LinguistTools REQUIRED)
find_package(Qt6Multimedia REQUIRED)
find_package(Qt6Widgets REQUIRED)
pkg_check_modules(FLAC flac)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)
pkg_check_modules(MAD mad)
pkg_check_modules(TAGLIB taglib)
pkg_check_modules(VORBIS vorbisfile)
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

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND BPMDETECT_CXX_WARN_ERROR_ALL)
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
  endif()
endif()

add_subdirectory(src)
if(BUILD_TESTS)
  find_package(Qt6Test REQUIRED)
  enable_testing()
  add_subdirectory(tests)
endif()
