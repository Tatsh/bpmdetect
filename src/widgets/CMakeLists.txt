set(WIDGETS_SRCS
    dlgbpmdetect.cpp
    dlgbpmdetect.h
    dlgbpmdetect.ui
    dlgtestbpm.cpp
    dlgtestbpm.h
    dlgtestbpmdlg.ui
    progressbar.cpp
    progressbar.h
    qdroplistview.cpp
    qdroplistview.h
    trackitem.cpp
    trackitem.h)
add_library(bpmdetect-widgets STATIC ${WIDGETS_SRCS})
target_include_directories(bpmdetect-widgets PRIVATE ..)
target_link_libraries(bpmdetect-widgets PRIVATE Qt6::Core Qt6::Gui Qt6::Multimedia Qt6::Widgets
                                                PkgConfig::SOUNDTOUCH)
# https://stackoverflow.com/a/60203765/374110
get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_isMultiConfig)
  set(AUTOGEN_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/bpmdetect-widgets_autogen/include_$<CONFIG>)
else()
  set(AUTOGEN_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/bpmdetect-widgets_autogen/include)
endif()
target_include_directories(bpmdetect-widgets INTERFACE $<BUILD_INTERFACE:${AUTOGEN_INCLUDE_DIR}>)
