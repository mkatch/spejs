cmake_path(SET PROTO_SRC_DIR "${PKG_SRC_DIR}/proto")
cmake_path(SET PROTO_OUT_DIR "${PKG_BUILD_DIR}/proto")
add_proto_generate_target(
  universe_proto
  SRC_DIR "${PROTO_SRC_DIR}"
  OUT_DIR "${PROTO_OUT_DIR}"
  LANGS "cpp" "go"
  GO_PACKAGE "github.com/mkacz91/spejs/universe"
)

file(GLOB SHADER_SRCS "${PKG_SRC_DIR}/shaders/*.glsl")
cmake_path(SET SHADER_SOURCES_H "${PKG_BUILD_DIR}/shaders/shader_sources.h")
bundle_shaders_cpp(
  SHADER_GEN_FILES
  ${SHADER_SOURCES_H}
  ${SHADER_SRCS}
)

file(GLOB UNIVERSE_SERVER_SRCS "${PKG_SRC_DIR}/*.cpp" "${PKG_SRC_DIR}/*.h")
add_executable(universe_server
  ${UNIVERSE_SERVER_SRCS}
  ${PROTO_CPP_GEN_FILES}
  ${SHADER_GEN_FILES}
)
target_link_libraries(universe_server
  ${GRPC_LIBS}
  glm
  gl_cpp
  glfw
  OpenGL::GL
  proto_cpp
  universe_proto_cpp
)
