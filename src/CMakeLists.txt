add_subdirectory(track)
add_subdirectory(widgets)

set(SOURCES images.h main.cpp)

ecm_qt_declare_logging_category(
  SOURCES
  HEADER
  generated/bpmdetect-debug.h
  IDENTIFIER
  "LOG_BPMDETECT"
  CATEGORY_NAME
  "sh.tat.${CMAKE_PROJECT_NAME}")

qt_add_executable(
  bpmdetect
  WIN32
  bpmdetect.qrc
  icons/16x16.png
  icons/16x16@2x.png
  icons/32x32.png
  icons/32x32@2x.png
  icons/48x48.png
  icons/48x48@2x.png
  icons/64x64.png
  icons/64x64@2x.png
  icons/96x96.png
  icons/128x128.png
  icons/128x128@2x.png
  icons/256x256.png
  icons/256x256@2x.png
  icons/512x512.png
  icons/bpmdetect.icns
  icons/bpmdetect.ico
  main.cpp
  utils.h)
if(WIN32)
  configure_file("version.rc.in" version.rc @ONLY)
  target_sources(bpmdetect PRIVATE version.rc)
elseif(APPLE AND (NOT FHS OR NON_PORTABLE_MACOS_BUNDLE))
  set_target_properties(
    bpmdetect
    PROPERTIES OUTPUT_NAME "BPM Detect"
               MACOSX_BUNDLE TRUE
               MACOSX_BUNDLE_BUNDLE_NAME "BPM Detect"
               MACOSX_BUNDLE_ICON_FILE "bpmdetect.icns"
               MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
               MACOSX_BUNDLE_GUI_IDENTIFIER "sh.tat.${CMAKE_PROJECT_NAME}"
               MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2025 BPM Detect authors."
               MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")
endif()
target_include_directories(bpmdetect PRIVATE . widgets)
target_link_libraries(bpmdetect PRIVATE PkgConfig::SOUNDTOUCH PkgConfig::TAGLIB Qt6::Gui
                                        Qt6::Widgets bpmdetect-track bpmdetect-widgets)
if(NOT FHS AND NOT NON_PORTABLE_MACOS_BUNDLE)
  if(APPLE)
    set(deploy_tool_options_arg -hardened-runtime)
    set(executable_path "$<TARGET_FILE_NAME:bpmdetect>.app")
    set(executable_dir .)
  elseif(WIN32)
    set(deploy_tool_options_arg --no-compiler-runtime)
    set(executable_path "$<TARGET_FILE_NAME:bpmdetect>")
    set(executable_dir "${CMAKE_INSTALL_BINDIR}")
  endif()
  if((APPLE AND NOT NON_PORTABLE_MACOS_BUNDLE) OR WIN32)
    qt_generate_deploy_script(
      TARGET
      bpmdetect
      OUTPUT_SCRIPT
      deploy_script
      CONTENT
      [[
    qt_deploy_runtime_dependencies(
    EXECUTABLE "${executable_dir}/${executable_path}"
    DEPLOY_TOOL_OPTIONS ${deploy_tool_options_arg}
    GENERATE_QT_CONF
    VERBOSE)
    ]])
  endif()
  install(TARGETS bpmdetect RUNTIME_DEPENDENCY_SET bpmdetect-deps BUNDLE DESTINATION .)
  if((APPLE AND NOT NON_PORTABLE_MACOS_BUNDLE) OR WIN32)
    install(SCRIPT ${deploy_script})
  endif()
  if(APPLE
     AND NOT FHS
     AND NOT NON_PORTABLE_MACOS_BUNDLE)
    install(FILES ../LICENSE.txt ../README.md ../CHANGELOG.md ../SECURITY.md ../CITATION.cff
                  icons/bpmdetect.icns
            DESTINATION "$<TARGET_FILE_NAME:bpmdetect>.app/Contents/Resources")
  else()
    install(FILES ../LICENSE.txt ../README.md ../CHANGELOG.md ../SECURITY.md TYPE DOC)
    install(FILES ../CITATION.cff DESTINATION ${CMAKE_INSTALL_DATADIR}/bpmdetect)
  endif()
  if(MINGW)
    get_filename_component(_mingw_bin_dir ${CMAKE_C_COMPILER} DIRECTORY)
    install(
      RUNTIME_DEPENDENCY_SET
      bpmdetect-deps
      PRE_EXCLUDE_REGEXES
      [=[api-ms-]=]
      [=[ext-ms-]=]
      [[kernel32\.dll]]
      POST_EXCLUDE_REGEXES
      [=[.*system32\/.*\.dll]=]
      DIRECTORIES
      ${_mingw_bin_dir}
      $<TARGET_FILE_DIR:bpmdetect>)
    install(
      FILES ${_mingw_bin_dir}/libaom.dll
            ${_mingw_bin_dir}/libLerc.dll
            ${_mingw_bin_dir}/libbrotlienc.dll
            ${_mingw_bin_dir}/libdeflate.dll
            ${_mingw_bin_dir}/libgme.dll
            ${_mingw_bin_dir}/libgsm.dll
            ${_mingw_bin_dir}/libhwy.dll
            ${_mingw_bin_dir}/libjxl.dll
            ${_mingw_bin_dir}/libjxl_cms.dll
            ${_mingw_bin_dir}/libjxl_threads.dll
            ${_mingw_bin_dir}/librav1e.dll
            ${_mingw_bin_dir}/libsoxr.dll
            ${_mingw_bin_dir}/libsrt.dll
            ${_mingw_bin_dir}/libssh.dll
            ${_mingw_bin_dir}/libva.dll
            ${_mingw_bin_dir}/libva_win32.dll
            ${_mingw_bin_dir}/xvidcore.dll
      DESTINATION ${CMAKE_INSTALL_BINDIR}
      OPTIONAL)
    install(
      DIRECTORY ${_mingw_bin_dir}
      DESTINATION .
      FILES_MATCHING
      REGEX
        "lib(jbig|datrie|expat|fribidi|gmodule-2.0|jpeg|lcms2|pangoft2-1.0|pangowin32-1.0|sharpyuv|\
        thai|tiff|bluray|cairo|cairo-gobject|crypto|dav1d|ffi|fontconfig|gdk_pixbuf-2.0|gio-2.0|\
        gmp|gnutls|gobject-2.0|gomp|hogweed|idn2|lc3|lzma|modplug|mp3lame|nettle|opencore-amr[nw]b|\
        openjp2|opus|p11-kit|pango-1.0|pangocairo-1.0|pixman-1|rsvg-2|rtmp|speex|SvtAv1Enc|tasn1|\
        theora(dec|enc)|unistring|vorbisenc|vpl|vpx|webp|webpmux|x26[45]|xml2|zvbi)-.*\.dll")
  endif()
else()
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/bpmdetect.desktop.in"
                 "${CMAKE_CURRENT_BINARY_DIR}/bpmdetect.desktop" @ONLY)
  install(TARGETS bpmdetect)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/bpmdetect.desktop"
          DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
  foreach(
    size
    16
    32
    48
    64
    96
    128
    256
    512)
    install(
      FILES "${CMAKE_CURRENT_SOURCE_DIR}/icons/${size}x${size}.png"
      DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/${size}x${size}/apps"
      RENAME bpmdetect.png)
  endforeach()
  install(FILES ../README.md TYPE DOC)
endif()

if(NOT APPLE)
  set(CPACK_GENERATOR ZIP)
elseif(APPLE)
  set(CPACK_GENERATOR DragNDrop ZIP)
endif()
set(CPACK_PACKAGE_VENDOR Tatsh)
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/LICENSE.txt)
set(__cpack_system_name ${CMAKE_SYSTEM_NAME})
if(__cpack_system_name MATCHES "Windows")
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(__cpack_system_name win64)
  else()
    set(__cpack_system_name win32)
  endif()
elseif(APPLE)
  if(CMAKE_APPLE_SILICON_PROCESSOR)
    set(__cpack_system_name Darwin-arm64)
  else()
    set(__cpack_system_name Darwin-x86_64)
  endif()
endif()
set(CPACK_PACKAGE_FILE_NAME
    ${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}-${__cpack_system_name}-${CMAKE_BUILD_TYPE})
include(CPack)
