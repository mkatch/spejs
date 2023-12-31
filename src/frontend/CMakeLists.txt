project(SpejsFrontend)

set(SRC_DIR ${PROJECT_SOURCE_DIR})
set(BIN_DIR ${PROJECT_BINARY_DIR})

add_custom_target(
  frontend_server ALL
  COMMAND go build
  -C "${SRC_DIR}"
  -o "${BIN_DIR}/frontend_server${CMAKE_EXECUTABLE_SUFFIX}"
)
add_dependencies(frontend_server
  generate_go_protos
  client_dist
)