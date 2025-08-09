set(CORE_SRCS
    audioanalyzer.cpp
    audioanalyzer.h
    audioinput.cpp
    audioinput.h
    beatinfo.cpp
    beatinfo.h
    bpmcalculator.cpp
    bpmcalculator.h
    bpmcounter.cpp
    bpmcounter.h
    BPMDetect.cpp
    BPMDetect.h
    energybeatdetector.cpp
    energybeatdetector.h
    FIFOSampleBuffer.cpp
    FIFOSampleBuffer.h
    FIFOSamplePipe.h
    metronome.cpp
    metronome.h
    pasample.h
    PeakFinder.cpp
    PeakFinder.h
    STTypes.h
    waveform.cpp
    waveform.h)
add_library(bpmdetect-core STATIC ${CORE_SRCS})
target_include_directories(bpmdetect-core PRIVATE ..)
target_link_libraries(bpmdetect-core PRIVATE ${PORTAUDIO_LIBRARIES} Qt6::Core bpmdetect-kissfft)
