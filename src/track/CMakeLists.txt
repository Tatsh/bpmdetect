set(TRACK_SRCS abstractbpmdetector.cpp abstractbpmdetector.h soundtouchbpmdetector.cpp
               soundtouchbpmdetector.h track.cpp track.h)
add_library(bpmdetect-track STATIC ${TRACK_SRCS})
target_include_directories(bpmdetect-track PRIVATE ..)
target_link_libraries(bpmdetect-track PRIVATE PkgConfig::SOUNDTOUCH Qt6::Core
                                              PkgConfig::FFMPEG)
target_compile_definitions(bpmdetect-track PRIVATE SAMPLE_MAX_VALUE=32768)
