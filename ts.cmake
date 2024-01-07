# TypeScript compiler always processes the entire project directory (the one
# containing the root tsconfig.json) and cannot conveniently be invoked for
# individual subdirectories. All products have to be written into a single
# directory (`JS_BUILD_DIR`), with structure mirroring that of the original
# project. 
#
# We have one global custom command that invokes tsc, but the targets can set
# precise dependencies on the files they need. The `extract_ts_files` function
# takes the .ts files and tells you what .js files they will produce. 
cmake_path(SET JS_BUILD_DIR "${BUILD_DIR}/js")

# The tsc tracks its own dependencies and only recompiles what has changed, so
# technically we only need to track the outputs. But it still takes a few
# seconds, which is annoying, so we also track the inputs to not even invoke
# tsc if not needed.
define_property(GLOBAL PROPERTY TSC_IN)
define_property(GLOBAL PROPERTY TSC_OUT)
function(append_global_tsc_inout TSC_IN_var TSC_OUT_var)
  set_property(GLOBAL APPEND PROPERTY TSC_IN ${${TSC_IN_var}})
  set_property(GLOBAL APPEND PROPERTY TSC_OUT ${${TSC_OUT_var}})
endfunction()

# Removes all .ts files from the given `FILES_var` list, marks them as
# a dependency for the TypeScript compiler (tsc), and returns the products in
# `TSC_OUT_outvar`.
function(extract_ts_files FILES_var TSC_OUT_outvar)
  set(TSC_IN "")
  set(TSC_OUT "")
  set(NON_TS_FILES "")

  foreach(FILE IN LISTS ${FILES_var})
    cmake_path(GET FILE EXTENSION EXT)
    get_relative_path_or_die(${FILE} REL_FILE)
    if(EXT STREQUAL ".ts" OR EXT STREQUAL ".d.ts")
      list(APPEND TSC_IN ${FILE})
      if(EXT STREQUAL ".ts")
        cmake_path(SET TSC_OUT_STEM "${JS_BUILD_DIR}/${REL_FILE}")
        cmake_path(REMOVE_EXTENSION TSC_OUT_STEM)
        list(APPEND TSC_OUT
          "${TSC_OUT_STEM}.js"
          # "${TSC_OUT_STEM}.js.map"
        )
      endif()
    else()
      list(APPEND NON_TS_FILES ${FILE})
    endif()
  endforeach()

  set(${FILES_var} ${NON_TS_FILES} PARENT_SCOPE)
  set(${TSC_OUT_outvar} ${TSC_OUT} PARENT_SCOPE)
  append_global_tsc_inout(TSC_IN TSC_OUT)
endfunction()

# Adds a custom command that invokes the TypeScript compiler. Needs to be called
# at the end of the root CMakeLists.txt file, after all `TSC_IN` and `TSC_OUT`
# have been collected.
function(add_tsc_command)
  get_property(TSC_IN GLOBAL PROPERTY TSC_IN)
  get_property(TSC_OUT GLOBAL PROPERTY TSC_OUT)

  add_custom_command(
    COMMENT "[ts] Compile TypesScript (tsc)"
    OUTPUT ${TSC_OUT}
    COMMAND npx tsc
      --rootDir "${ROOT_DIR}"
      --outDir "${JS_BUILD_DIR}"
    DEPENDS ${TSC_IN}
  )

  add_custom_target(ts_compile DEPENDS ${TSC_OUT})
endfunction()
