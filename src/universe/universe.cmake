

file(GLOB SHADER_SRCS "${PKG_SRC_DIR}/shaders/*.glsl")
cmake_path(SET SHADER_SOURCES_H "${PKG_BUILD_DIR}/shaders/shader_sources.h")
bundle_shaders_cpp(
  SHADER_GEN_FILES
  ${SHADER_SOURCES_H}
  ${SHADER_SRCS}
)

file(GLOB UNIVERSE_SERVER_SRCS "${PKG_SRC_DIR}/*.cpp")
add_executable(universe_server
  ${UNIVERSE_SERVER_SRCS}
  ${PROTO_CPP_GEN_FILES}
  ${SHADER_GEN_FILES}
)
target_link_libraries(universe_server
  ${GRPC_LIBS}
  glpp
  glfw
  OpenGL::GL
)