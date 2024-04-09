if(NOT DEFINED ecap5_treq_EXECUTABLE)
  message(STATUS "Collecting ECAP5-TREQ")
  # Download external dependencies
  FetchContent_Declare(ECAP5_TREQ
    GIT_REPOSITORY https://github.com/cchaine/ECAP5-TREQ  
    GIT_TAG tags/v2.1.1
  )
  FetchContent_MakeAvailable(ECAP5_TREQ)

  execute_process(COMMAND ${Python3_EXECUTABLE} -m pip install ${ecap5_treq_SOURCE_DIR} OUTPUT_QUIET RESULT_VARIABLE rv)
  if("${rv}" STREQUAL "0")
    set(ecap5_treq_EXECUTABLE ${VENV_DIR}/bin/ecap5-treq CACHE STRING "path to the ecap5-treq executable")
    message(STATUS "Collecting ECAP5-TREQ - Success")
  endif()
endif()

if(DEFINED ecap5_treq_EXECUTABLE)
  # Define commands for using ECAP5-TREQ
  add_custom_target(report
    DEPENDS ${TEST_OUTPUTS}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND ${ecap5_treq_EXECUTABLE} -c ${CMAKE_SOURCE_DIR}/config/treq.json gen_report -o ${CMAKE_BINARY_DIR}/report.html --html)
  add_custom_command(
    OUTPUT report.md
    DEPENDS ${TEST_OUTPUTS}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND ${ecap5_treq_EXECUTABLE} -c ${CMAKE_SOURCE_DIR}/config/treq.json gen_report -o ${CMAKE_BINARY_DIR}/report.md)
  add_custom_target(report_markdown DEPENDS report.md)
  add_custom_command(
    OUTPUT test-result-badge.json
    DEPENDS ${TEST_OUTPUTS}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND ${ecap5_treq_EXECUTABLE} -c ${CMAKE_SOURCE_DIR}/config/treq.json gen_test_result_badge -o ${CMAKE_BINARY_DIR}/test-result-badge.json)
  add_custom_command(
    OUTPUT traceability-result-badge.json
    DEPENDS ${TEST_OUTPUTS}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND ${ecap5_treq_EXECUTABLE} -c ${CMAKE_SOURCE_DIR}/config/treq.json gen_traceability_result_badge -o ${CMAKE_BINARY_DIR}/traceability-result-badge.json)
  add_custom_target(badges DEPENDS test-result-badge.json traceability-result-badge.json)

  add_custom_command(
    OUTPUT traceability-matrix.csv
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND ${ecap5_treq_EXECUTABLE} -c ${CMAKE_SOURCE_DIR}/config/treq.json prepare_matrix -o ${CMAKE_BINARY_DIR}/traceability-matrix.csv)
  add_custom_target(prepare_matrix DEPENDS traceability-matrix.csv)
else()
  message(WARNING "Failed to collect ECAP5-TREQ")
endif()
