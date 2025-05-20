include_directories(${Qt6Core_INCLUDE_DIRS}
                    ${Qt6Gui_INCLUDE_DIRS}
                    ${Qt6Multimedia_INCLUDE_DIRS}
                    ${Qt6Widgets_INCLUDE_DIRS}
                    ${TAGLIB_INCLUDE_DIRS}
                    ${PORTAUDIO_INCLUDE_DIRS}
                    ${FLAC_INCLUDE_DIRS}
                    ${MAD_INCLUDE_DIRS})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -fPIE -fPIC")

if (TAGLIB_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_TAGLIB")
endif()
if (VORBIS_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_VORBISFILE")
endif()
if (MAD_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_MAD")
endif()
if (FLAC_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_FLAC")
endif()

set(UI
    dlgbpmdetect.ui
    dlgrealtimedlg.ui
    dlgtestbpmdlg.ui
)

set(SOURCES
    audioanalyzer.cpp
    audioinput.cpp
    beatinfo.cpp
    bpmcalculator.cpp
    bpmcounter.cpp
    BPMDetect.cpp
    dlgbpmdetect.cpp
    dlgrealtime.cpp
    dlgtestbpm.cpp
    dlgtestbpmplayer.cpp
    energybeatdetector.cpp
    FIFOSampleBuffer.cpp
    kiss_fft.c
    kiss_fftr.c
    main.cpp
    metronome.cpp
    PeakFinder.cpp
    progressbar.cpp
    qdroplistview.cpp
    track.cpp
    trackproxy.cpp
    trackwav.cpp
    waveform.cpp
    wbpmcalcdisplay.cpp
    wenergybeatdisplay.cpp
    wplot.cpp
    wvumeter.cpp
    wwaveform.cpp
)

# TODO Realtime sources
# sources_realtime="""
# audioinput.cpp
# audioanalyzer.cpp
# kiss_fft.c
# kiss_fftr.c
# energybeatdetector.cpp
# bpmcounter.cpp
# beatinfo.cpp
# bpmcalculator.cpp
# metronome.cpp
# waveform.cpp
# """
# sources_realtime_gui="""
# dlgrealtime.cpp
# wvumeter.cpp
# wwaveform.cpp
# wenergybeatdisplay.cpp
# wplot.cpp
# wbpmcalcdisplay.cpp
# """
# realtime_sources="""
# mainreal.cpp
# PeakFinder.cpp
# """
# sources_realtime_ui="""
# dlgrealtimedlg.ui
# """
# TODO Support for Windows

if(FLAC_FOUND)
    set(FLAC_SOURCES trackflac.cpp)
endif()
if(MAD_FOUND)
    set(MP3_SOURCES trackmp3.cpp)
endif()
if (VORBIS_FOUND)
    set(VORBIS_SOURCES trackoggvorbis.cpp)
endif()

add_executable(bpmdetect
  MACOSX_BUNDLE
  ${BPMDETECT-WIN32-FLAG}
  ${BPMDETECT-WIN32-RESOURCES}
  ${FLAC_SOURCES}
  ${MP3_SOURCES}
  ${VORBIS_SOURCES}
  ${SOURCES}
)
# Temporary workaround until .ui file is fixed or connections are done in the code
set_property(TARGET bpmdetect PROPERTY AUTOUIC_OPTIONS "--connections;string")
target_link_libraries(bpmdetect
    ${PORTAUDIO_LIBRARIES}
    ${FLAC_LIBRARIES}
    ${MAD_LIBRARIES}
    ${VORBIS_LIBRARIES}
    ${TAGLIB_LIBRARIES}
    Qt6::Multimedia
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)
