set(CORE_DIR ../../src/core)

set(METRONOME_TESTS_SRCS metronometest.cpp ${CORE_DIR}/metronome.cpp ${CORE_DIR}/metronome.h)
create_test(metronome-test "${METRONOME_TESTS_SRCS}")
set(WAVEFORM_TEST_SRCS waveformtest.cpp ${CORE_DIR}/waveform.cpp ${CORE_DIR}/waveform.h)
create_test(waveform-test "${WAVEFORM_TEST_SRCS}")
