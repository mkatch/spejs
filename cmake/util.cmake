# Converts an absolute path to be relative the project root directory and
# panics if it cannot do so.
#
# Panic is because if you are passing a wrong path, then your build will 99.9%
# be messed up, so it's best to detect early.
function(get_relative_path_or_die ABS_PATH REL_PATH_outvar)
  cmake_path(RELATIVE_PATH ABS_PATH OUTPUT_VARIABLE REL_PATH)
  if (REL_PATH STREQUAL "")
    message(FATAL_ERROR
      "Path '${ABS_PATH}' is not an absolute path under project root "
      "(${CMAKE_SOURCE_DIR})")
  endif()
  set(${REL_PATH_outvar} ${REL_PATH} PARENT_SCOPE)
endfunction()