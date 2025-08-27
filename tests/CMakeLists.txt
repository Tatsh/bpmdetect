# SPDX-License-Identifier: MIT
function(create_test test_name test_srcs)
  ecm_qt_declare_logging_category(
    test_srcs
    HEADER
    ../src/debug.h
    IDENTIFIER
    "gLogBpmDetect"
    CATEGORY_NAME
    "sh.tat.${CMAKE_PROJECT_NAME}")
  add_executable(${test_name} ${test_srcs})
  target_compile_definitions(${test_name} PRIVATE TESTING)
  target_compile_options(${test_name} PRIVATE ${DEBUG_FLAGS})
  target_include_directories(${test_name} PRIVATE ../src ../../src)
  target_link_options(${test_name} PRIVATE ${DEBUG_FLAGS})
  target_link_libraries(${test_name} PRIVATE Qt::Core Qt::Test)
  add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

set(UTILS_TESTS_SRCS utilstest.cpp ../src/utils.cpp ../src/utils.h)
create_test(utils-test "${UTILS_TESTS_SRCS}")

set(TRACK_TESTS_SRCS
    track/5s-silent-artist-title.mp3
    track/tracktest.cpp
    ../src/track/abstractbpmdetector.cpp
    ../src/track/abstractbpmdetector.h
    ../src/track/soundtouchbpmdetector.cpp
    ../src/track/soundtouchbpmdetector.h
    ../src/track/track.cpp
    ../src/ffmpegutils.cpp
    ../src/ffmpegutils.h
    ../src/utils.cpp
    ../src/utils.h)
create_test(track-test "${TRACK_TESTS_SRCS}")
target_compile_definitions(
  track-test
  PRIVATE TEST_FILE_5S_SILENT=\"${CMAKE_CURRENT_SOURCE_DIR}/track/5s-silent-artist-title.mp3\")
target_link_libraries(track-test PRIVATE PkgConfig::FFMPEG PkgConfig::SOUNDTOUCH Qt6::Multimedia)

set(WIDGETS_TESTS_SRCS widgets/qdroplistviewtest.cpp ../src/widgets/qdroplistview.cpp
                       ../src/widgets/qdroplistview.h)
create_test(qdroplistview-test "${WIDGETS_TESTS_SRCS}")
target_link_libraries(qdroplistview-test PRIVATE Qt::Widgets)

set(PROGRESSBAR_TESTS_SRCS widgets/progressbartest.cpp ../src/widgets/progressbar.cpp
                           ../src/widgets/progressbar.h)
create_test(progressbar-test "${PROGRESSBAR_TESTS_SRCS}")
target_link_libraries(progressbar-test PRIVATE Qt::Widgets)

set(DLGTESTBPMPLAYER_TESTS_SRCS
    widgets/dlgtestbpmplayertest.cpp ../src/widgets/dlgtestbpmplayer.cpp
    ../src/widgets/dlgtestbpmplayer.h noise.wav 140bpm.ogg)
create_test(dlgtestbpmplayer-test "${DLGTESTBPMPLAYER_TESTS_SRCS}")
target_compile_definitions(
  dlgtestbpmplayer-test PRIVATE NOISE_WAV=\"${CMAKE_CURRENT_SOURCE_DIR}/noise.wav\"
                                TEST_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/140bpm.ogg\")
target_link_libraries(dlgtestbpmplayer-test PRIVATE Qt::Widgets Qt::Multimedia)

set(DLGTESTBPM_TESTS_SRCS
    noise.wav
    widgets/dlgtestbpmtest.cpp
    ../src/utils.cpp
    ../src/utils.h
    ../src/widgets/dlgtestbpm.cpp
    ../src/widgets/dlgtestbpm.h
    ../src/widgets/dlgtestbpmplayer.cpp
    ../src/widgets/dlgtestbpmplayer.h
    ../src/widgets/progressbar.cpp
    ../src/widgets/progressbar.h)
create_test(dlgtestbpm-test "${DLGTESTBPM_TESTS_SRCS}")
target_compile_definitions(dlgtestbpm-test
                           PRIVATE NOISE_WAV=\"${CMAKE_CURRENT_SOURCE_DIR}/noise.wav\")
target_link_libraries(dlgtestbpm-test PRIVATE PkgConfig::SOUNDTOUCH Qt::Multimedia Qt::Widgets)

set(DLGBPMDETECT_TESTS_SRCS
    140bpm.ogg
    widgets/dlgbpmdetecttest.cpp
    ../src/ffmpegutils.cpp
    ../src/ffmpegutils.h
    ../src/utils.cpp
    ../src/utils.h
    ../src/track/abstractbpmdetector.cpp
    ../src/track/abstractbpmdetector.h
    ../src/track/soundtouchbpmdetector.cpp
    ../src/track/soundtouchbpmdetector.h
    ../src/track/track.cpp
    ../src/track/track.h
    ../src/widgets/dlgbpmdetect.cpp
    ../src/widgets/dlgbpmdetect.h
    ../src/widgets/dlgtestbpm.cpp
    ../src/widgets/dlgtestbpm.h
    ../src/widgets/dlgtestbpmplayer.cpp
    ../src/widgets/dlgtestbpmplayer.h
    ../src/widgets/progressbar.cpp
    ../src/widgets/progressbar.h
    ../src/widgets/qdroplistview.cpp
    ../src/widgets/qdroplistview.h
    ../src/widgets/trackitem.cpp
    ../src/widgets/trackitem.h)
create_test(dlgbpmdetect-test "${DLGBPMDETECT_TESTS_SRCS}")
target_compile_definitions(
  dlgbpmdetect-test PRIVATE SAMPLE_MAX_VALUE=32768
                            TEST_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/140bpm.ogg\")
target_link_libraries(dlgbpmdetect-test PRIVATE PkgConfig::FFMPEG PkgConfig::SOUNDTOUCH
                                                Qt::Multimedia Qt::Widgets)

set(CONSOLEMAIN_TESTS_SRCS
    consolemaintest.cpp
    140bpm.ogg
    ../src/consolemain.cpp
    ../src/consolemain.h
    ../src/track/track.cpp
    ../src/track/track.h
    ../src/track/abstractbpmdetector.cpp
    ../src/track/abstractbpmdetector.h
    ../src/track/soundtouchbpmdetector.cpp
    ../src/track/soundtouchbpmdetector.h
    ../src/ffmpegutils.cpp
    ../src/ffmpegutils.h
    ../src/utils.cpp
    ../src/utils.h)
create_test(consolemain-test "${CONSOLEMAIN_TESTS_SRCS}")
target_link_libraries(consolemain-test PRIVATE PkgConfig::FFMPEG PkgConfig::SOUNDTOUCH
                                               Qt::Multimedia)
target_compile_definitions(consolemain-test
                           PRIVATE TEST_FILE_140BPM=\"${CMAKE_CURRENT_SOURCE_DIR}/140bpm.ogg\")
