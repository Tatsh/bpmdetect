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
    wwaveform.cpp)

if (FLAC_FOUND)
  set(FLAC_SOURCES trackflac.cpp)
endif ()
if (MAD_FOUND)
  set(MP3_SOURCES trackmp3.cpp)
endif ()
if (VORBIS_FOUND)
  set(VORBIS_SOURCES trackoggvorbis.cpp)
endif ()

add_executable(bpmdetect MACOSX_BUNDLE ${BPMDETECT-WIN32-FLAG} ${BPMDETECT-WIN32-RESOURCES}
                                       ${FLAC_SOURCES} ${MP3_SOURCES} ${VORBIS_SOURCES} ${SOURCES})
target_compile_definitions(
  bpmdetect
  PRIVATE $<$<BOOL:${FLAC_FOUND}>:HAVE_FLAC> $<$<BOOL:${MAD_FOUND}>:HAVE_MAD>
          $<$<BOOL:${VORBIS_FOUND}>:HAVE_VORBISFILE> $<$<BOOL:${TAGLIB_FOUND}>:HAVE_TAGLIB>)
# Temporary workaround until .ui file is fixed or connections are done in the code
set_property(TARGET bpmdetect PROPERTY AUTOUIC_OPTIONS "--connections;string")
target_include_directories(bpmdetect PRIVATE ${TAGLIB_INCLUDE_DIRS} ${PORTAUDIO_INCLUDE_DIRS}
                                             ${FLAC_INCLUDE_DIRS} ${MAD_INCLUDE_DIRS} .)
target_link_libraries(
  bpmdetect
  ${PORTAUDIO_LIBRARIES}
  ${FLAC_LIBRARIES}
  ${MAD_LIBRARIES}
  ${VORBIS_LIBRARIES}
  ${TAGLIB_LIBRARIES}
  Qt6::Multimedia
  Qt6::Core
  Qt6::Gui
  Qt6::Widgets)
