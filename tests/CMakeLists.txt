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
  target_compile_options(
    ${test_name} PRIVATE ${DEBUG_FLAGS}
                         $<$<AND:$<CXX_COMPILER_ID:Clang,GNU>,$<BOOL:${COVERAGE}>>:--coverage>)
  target_include_directories(${test_name} PRIVATE ../src ../../src)
  target_link_options(${test_name} PRIVATE ${DEBUG_FLAGS}
                      $<$<AND:$<CXX_COMPILER_ID:Clang>,$<BOOL:${COVERAGE}>>:--coverage>)
  target_link_libraries(${test_name}
                        PRIVATE Qt6::Test $<$<AND:$<CXX_COMPILER_ID:GNU>,$<BOOL:${COVERAGE}>>:gcov>)
  add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

add_subdirectory(core)
