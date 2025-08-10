add_subdirectory(core)
add_subdirectory(track)
add_subdirectory(widgets)

set(SOURCES images.h main.cpp)

ecm_qt_declare_logging_category(
  SOURCES
  HEADER
  generated/bpmdetectdebug.h
  IDENTIFIER
  "LOG_BPMDETECT"
  CATEGORY_NAME
  "sh.tat.${CMAKE_PROJECT_NAME}")

add_executable(bpmdetect MACOSX_BUNDLE icon.rc images.h main.cpp)
target_include_directories(bpmdetect PRIVATE . widgets)
target_link_libraries(
  bpmdetect
  PkgConfig::SOUNDTOUCH
  PkgConfig::TAGLIB
  Qt6::Gui
  Qt6::Widgets
  bpmdetect-core
  bpmdetect-track
  bpmdetect-widgets
  kissfft::kissfft)
qt_add_translations(bpmdetect MERGE_QT_TRANSLATIONS QM_FILES_OUTPUT_VARIABLE qm_files TS_FILES
                    ../i18n/bpmdetect_en.ts)
if(I18N)
  install(FILES ${qm_files} DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/bpmdetect")
endif()
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/bpmdetect.desktop.in"
               "${CMAKE_CURRENT_BINARY_DIR}/bpmdetect.desktop" @ONLY)
install(TARGETS bpmdetect)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/bpmdetect.desktop"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/bpmdetect.png"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/32x32/apps")
