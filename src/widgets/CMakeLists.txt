set(WIDGETS_SRCS
    dlgbpmdetect.cpp
    dlgbpmdetect.h
    dlgtestbpm.cpp
    dlgtestbpm.h
    dlgtestbpmplayer.cpp
    dlgtestbpmplayer.h
    progressbar.cpp
    progressbar.h
    qdroplistview.cpp
    qdroplistview.h)
add_library(bpmdetect-widgets STATIC ${WIDGETS_SRCS})
# Temporary workaround until .ui file is fixed or connections are done in the code.
set_property(TARGET bpmdetect-widgets PROPERTY AUTOUIC_OPTIONS "--connections;string")
target_compile_definitions(
  bpmdetect-widgets
  PRIVATE $<$<BOOL:${FLAC_FOUND}>:HAVE_FLAC> $<$<BOOL:${MAD_FOUND}>:HAVE_MAD>
          $<$<BOOL:${VORBIS_FOUND}>:HAVE_VORBISFILE> $<$<BOOL:${TAGLIB_FOUND}>:HAVE_TAGLIB>
          $<$<BOOL:${WAVPACK_FOUND}>:HAVE_WAVPACK>)
target_include_directories(bpmdetect-widgets PRIVATE ..)
target_link_libraries(bpmdetect-widgets PRIVATE bpmdetect-core Qt6::Core Qt6::Gui Qt6::Multimedia
                                                Qt6::Widgets PkgConfig::SOUNDTOUCH)
# https://stackoverflow.com/a/60203765/374110
get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_isMultiConfig)
  set(AUTOGEN_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/bpmdetect-widgets_autogen/include_$<CONFIG>)
else()
  set(AUTOGEN_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/bpmdetect-widgets_autogen/include)
endif()
target_include_directories(bpmdetect-widgets INTERFACE $<BUILD_INTERFACE:${AUTOGEN_INCLUDE_DIR}>)
