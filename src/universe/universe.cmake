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

file(GLOB UNIVERSE_SERVER_SRCS "${PKG_SRC_DIR}/*.cpp")
add_executable(universe_server
  ${UNIVERSE_SERVER_SRCS}
  ${PROTO_CPP_GEN_FILES}
  ${GLAD_GEN_FILES}
)
target_include_directories(universe_server
  PRIVATE ${GLAD_INCLUDE_DIR}
)
target_link_libraries(universe_server
  ${GRPC_LIBS}
  glfw
  OpenGL::GL
)