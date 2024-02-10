cmake_path(SET GLAD_OUT_DIR "${PKG_BUILD_DIR}/glad")
cmake_path(SET GLAD_INCLUDE_DIR "${GLAD_OUT_DIR}/include")

set(GLAD_GEN_FILES
  "${GLAD_INCLUDE_DIR}/glad/gl.h"
  "${GLAD_OUT_DIR}/src/gl.c"
)
add_custom_command(
  COMMENT "Generating OpenGL loader with glad"
  OUTPUT ${GLAD_GEN_FILES}
  COMMAND ${GLAD}
    --api gl:core=4.6
    --extensions " "
    --out-path "${GLAD_OUT_DIR}"
    c
)

include_directories(${GLAD_INCLUDE_DIR})

file(GLOB GLPP_SRCS "${PKG_SRC_DIR}/*.cpp" "${PKG_SRC_DIR}/*.h")
add_library(glpp
  ${GLPP_SRCS}
  ${GLAD_GEN_FILES}
)

cmake_path(GET PKG_SRC_DIR PARENT_PATH GLPP_INCLUDE_DIR)
include_directories(${GLPP_INCLUDE_DIR})