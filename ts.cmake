# TypeScript compiler always processes the entire project directory (the one
# containing the root tsconfig.json) and cannot conveniently be invoked for
# individual subdirectories. All products have to be written into a single
# directory (`JS_BUILD_DIR`), with a structure mirroring that of the original
# project. 
#
# We have one global custom command that invokes tsc and since it tracks its
# own dependencies, only rebuilding what is needed, we could just run it
# unconditionally. But it still takes a few seconds, even without changes, so we
# chose to track dependencies anyway. During configuration, all modules should
# submit their files or trgets that should cause the tsc to run
# (`ts_add_dependencies`), and call the `ts_add_command` to add the custom
# command with all the dependencies.

# The output directory of the TypeScript compiler.
cmake_path(SET JS_BUILD_DIR "${BUILD_DIR}/js")


define_property(GLOBAL PROPERTY TSC_DEPS)

# Adds dependencies for the TypeScript compiler custom command
#
# Only add non-generated files or targets. If the .ts file that directly
# contributes to the output is generated, add the original source file and the
# generating target instead as a dependency. The compiler will be invoked only
# if any of the files change. The target dependencies will be triggered before
# invoking the command.
function (ts_add_dependencies)
  set_property(GLOBAL APPEND PROPERTY TSC_DEPS ${ARGV})
endfunction()

# For an absolute path to a .ts file, which is an input to the TypeScript
# compiler, returns the absolute path to the resulting .js file.
function (ts_get_corresponding_js TS_FILE JS_FILE_outvar)
  cmake_path(GET TS_FILE EXTENSION TS_EXT)
  if (NOT TS_EXT STREQUAL ".ts")
    message(FATAL_ERROR "Expected a .ts file, got ${TS_FILE}")
  endif()
  get_relative_path_or_die(${TS_FILE} REL_FILE)
  cmake_path(REMOVE_EXTENSION REL_FILE)
  cmake_path(SET JS_FILE "${JS_BUILD_DIR}/${REL_FILE}.js")
  set(${JS_FILE_outvar} ${JS_FILE} PARENT_SCOPE)
endfunction()

# Adds a custom command that invokes the TypeScript compiler. Needs to be called
# at the end of the root CMakeLists.txt file, after all dependencies have been
# collected using `ts_add_dependencies`.
function(ts_add_command)
  cmake_path(SET TSC_STAMP "${BUILD_DIR}/ts_compile.stamp")
  get_property(TSC_DEPS GLOBAL PROPERTY TSC_DEPS)

  add_custom_command(
    COMMENT "[ts] Compile TypesScript (tsc)"
    OUTPUT ${TSC_STAMP}
    COMMAND ${CMAKE_COMMAND} -E touch ${TSC_STAMP}
    COMMAND npx tsc
      --rootDir "${ROOT_DIR}"
      --outDir "${JS_BUILD_DIR}"
    DEPENDS
      ${TSCONFIG_JSON}
      ${TSC_DEPS}
  )

  add_custom_target(
    ts_compile
    COMMENT "[ts] TypeScript compiled files up to date."
    DEPENDS ${TSC_STAMP}
  )
endfunction()
