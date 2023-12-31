project(SpejsLauncher)

set(SRC_DIR ${PROJECT_SOURCE_DIR})
set(BINARY_DIR ${PROJECT_BINARY_DIR})

add_custom_target(
  launcher ALL
  COMMAND go build
  -C "${SRC_DIR}"
  -o "${BINARY_DIR}"
)
add_dependencies(launcher
  frontend_server
  universe_server
)