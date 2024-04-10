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
  message(WARNING "Could not find ECAP5-TREQ")
endif()
