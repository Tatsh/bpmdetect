set(CORE_DIR ../../src/core)

set(METRONOME_TESTS_SRCS metronometest.cpp ${CORE_DIR}/metronome.cpp ${CORE_DIR}/metronome.h)
create_test(metronome-test "${METRONOME_TESTS_SRCS}")
