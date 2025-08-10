set(CORE_SRCS
    audioanalyzer.cpp
    audioanalyzer.h
    beatinfo.cpp
    beatinfo.h
    bpmcalculator.cpp
    bpmcalculator.h
    bpmcounter.cpp
    bpmcounter.h
    energybeatdetector.cpp
    energybeatdetector.h
    metronome.cpp
    metronome.h
    pasample.h
    waveform.cpp
    waveform.h)
add_library(bpmdetect-core STATIC ${CORE_SRCS})
target_include_directories(bpmdetect-core PRIVATE ..)
target_link_libraries(bpmdetect-core PRIVATE Qt6::Core kissfft::kissfft PkgConfig::SOUNDTOUCH)
