# SPDX-License-Identifier: MIT
function(create_test test_name test_srcs)
  ecm_qt_declare_logging_category(
    SOURCES
    HEADER
    generated/bpmdetectdebug.h
    IDENTIFIER
    "LOG_BPMDETECT"
    CATEGORY_NAME
    "sh.tat.${CMAKE_PROJECT_NAME}")
  add_executable(${test_name} ${test_srcs})
  target_compile_definitions(${test_name} PRIVATE TESTING)
  target_compile_options(${test_name} PRIVATE ${DEBUG_FLAGS})
  target_include_directories(${test_name} PRIVATE ../src ../../src)
  if(COVERAGE AND CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(${test_name} PRIVATE --coverage)
    target_link_libraries(${test_name} PRIVATE gcov)
  endif()
  target_link_libraries(${test_name} PRIVATE Qt6::Test)
  add_test(NAME ${test_name} COMMAND ${test_name})
  # ecm_mark_as_test(${test_name})
endfunction()

add_subdirectory(core)
