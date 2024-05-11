find_package(Sphinx)

if(Sphinx_FOUND)
  add_sphinx_document(
      documentation
      CONF_FILE "${CMAKE_CURRENT_LIST_DIR}/src/conf.py"
      SKIP_PDF
      "${CMAKE_CURRENT_LIST_DIR}/src/_ext/requirement.py"
      "${CMAKE_CURRENT_LIST_DIR}/src/_static/css/custom.css"
      "${CMAKE_CURRENT_LIST_DIR}/src/assets/architecture.svg"
      "${CMAKE_CURRENT_LIST_DIR}/src/assets/logo.svg"
      "${CMAKE_CURRENT_LIST_DIR}/src/assets/traceability.svg"
      "${CMAKE_CURRENT_LIST_DIR}/src/assets/riscv-encoding.svg"
      "${CMAKE_CURRENT_LIST_DIR}/src/assets/riscv-opcode.svg"
      "${CMAKE_CURRENT_LIST_DIR}/src/assets/wishbone-read.svg"
      "${CMAKE_CURRENT_LIST_DIR}/src/assets/wishbone-write.svg"
      "${CMAKE_CURRENT_LIST_DIR}/src/assets/pipeline-stall.svg"
      "${CMAKE_CURRENT_LIST_DIR}/src/assets/pipeline-stall-timing.svg"
      "${CMAKE_CURRENT_LIST_DIR}/src/index.rst"
      "${CMAKE_CURRENT_LIST_DIR}/src/arch/index.rst"
      "${CMAKE_CURRENT_LIST_DIR}/src/arch/1_introduction.rst"
      "${CMAKE_CURRENT_LIST_DIR}/src/arch/2_overall-description.rst"
      "${CMAKE_CURRENT_LIST_DIR}/src/arch/3_requirements.rst"
      "${CMAKE_CURRENT_LIST_DIR}/src/arch/4_configuration.rst"
      "${CMAKE_CURRENT_LIST_DIR}/src/arch/5_architecture.rst"
    )
else()
  message(WARNING "No Sphinx found. Documentation target not available.")
endif()
