find_package(LATEX)

set(DOCS_TARGETS)
macro(add_document dir)
  set(LATEX_OUTPUT_PATH ${CMAKE_BINARY_DIR}/docs/${dir})
  file(MAKE_DIRECTORY ${LATEX_OUTPUT_PATH})

  set(WORKINGDIR "${CMAKE_CURRENT_SOURCE_DIR}")
  set(MAIN_TEX_FILENAME "main")
  set(MAIN_TEX "${WORKINGDIR}/${dir}/${MAIN_TEX_FILENAME}.tex")
  file(GLOB_RECURSE ALL_TEX "*.tex")

  # First pass only run once.
  set(PREBUILD_OUTPUTS ${LATEX_OUTPUT_PATH}/${MAIN_TEX_FILENAME}.aux
                       ${LATEX_OUTPUT_PATH}/${MAIN_TEX_FILENAME}.out
                       ${LATEX_OUTPUT_PATH}/${MAIN_TEX_FILENAME}.toc)
  add_custom_command(
    OUTPUT ${PREBUILD_OUTPUTS}
    COMMAND ${PDFLATEX_COMPILER} -output-directory ${LATEX_OUTPUT_PATH} -draftmode -interaction=nonstopmode ${MAIN_TEX}
    COMMENT "Starting Prebuild"
    WORKING_DIRECTORY ${WORKINGDIR}
    DEPENDS ${ALL_TEX})
  add_custom_target(docs-${dir}-prebuild DEPENDS ${PREBUILD_OUTPUTS})
  # Second pass - generate the final pdf
  add_custom_command(
    OUTPUT ${LATEX_OUTPUT_PATH}/${MAIN_TEX_FILENAME}.pdf
    COMMAND ${PDFLATEX_COMPILER} -output-directory ${LATEX_OUTPUT_PATH} ${MAIN_TEX}
    WORKING_DIRECTORY ${WORKINGDIR}
    COMMENT "Assembling the final pdf file."
    DEPENDS ${PREBUILD_OUTPUTS})
  add_custom_target(docs-${dir} DEPENDS ${LATEX_OUTPUT_PATH}/${MAIN_TEX_FILENAME}.pdf)
  # Add the target to the docs targets
  list(APPEND DOCS_TARGETS docs-${dir})
endmacro()

add_document(arch)

add_custom_target(docs DEPENDS ${DOCS_TARGETS})
