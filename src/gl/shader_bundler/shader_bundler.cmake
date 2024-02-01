file(GLOB_RECURSE SHADER_BUNDLER_SRCS
  "${PKG_SRC_DIR}/*.go"
  "${PKG_SRC_DIR}/go.mod"
)

cmake_path(SET SHADER_BUNDLER "${PKG_BUILD_DIR}/shader_bundler${CMAKE_EXECUTABLE_SUFFIX}")
set(SHADER_BUNDLER ${SHADER_BUNDLER} PARENT_SCOPE)

add_custom_command(
  COMMENT "[shader_bundler] Building ${SHADER_BUNDLER}"
  OUTPUT ${SHADER_BUNDLER}
  COMMAND go build -C "${PKG_SRC_DIR}" -o "${SHADER_BUNDLER}"
  DEPENDS ${SHADER_BUNDLER_SRCS}
)

add_custom_target(
  shader_bundler ALL
  COMMENT "[shader_bundler] ${SHADER_BUNDLER} up to date."
  DEPENDS ${SHADER_BUNDLER}
)

# Bundles .glsl files so that they can be used at strings in C++ code.
#
# Produces one header file, declaring a constant for each shader, and multiple
# .cpp files, one for each .glsl.
#
# The header is specified via HEADER, and the .glsl files as the following
# varargs. The source files must all be in the same directory and the
# corresponding .cpp files are generated in the same directory as the header.
# All the generated files, including the header, are returned via
# GEN_FILES_outvar.
function(bundle_shaders_cpp GEN_FILES_outvar HEADER)
  list(SUBLIST ARGV 2 -1 SRCS)
  cmake_path(GET HEADER PARENT_PATH OUT_DIR)
  set(GEN_FILES "")

  LIST(APPEND GEN_FILES ${HEADER})
  add_custom_command(
    COMMENT "[shader_bundler] Creating header file ${HEADER}"
    OUTPUT ${HEADER}
    COMMAND ${SHADER_BUNDLER} cppheader "${HEADER}" ${SRCS}
    DEPENDS shader_bundler ${SHADER_BUNDLER} ${SRCS}
  )

  foreach (SRC ${SRCS})
    cmake_path(GET SRC STEM NAME)
    set(OUTPUT "${OUT_DIR}/${NAME}.cpp")
    LIST(APPEND GEN_FILES ${OUTPUT})
    add_custom_command(
      COMMENT "[shader_bundler] Bundling ${SRC} into ${OUTPUT}"
      OUTPUT ${OUTPUT}
      COMMAND "${SHADER_BUNDLER}" cppsrc "${HEADER}" ${SRC}
      DEPENDS shader_bundler ${SHADER_BUNDLER} ${SRC}
    )
  endforeach()

  set(${GEN_FILES_outvar} ${GEN_FILES} PARENT_SCOPE)
endfunction()
