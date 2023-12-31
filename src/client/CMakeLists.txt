project(SpejsClient)

set(SRC_DIR ${PROJECT_SOURCE_DIR})
set(BINARY_DIR ${PROJECT_BINARY_DIR})

add_custom_target(
  client_dist ALL
  COMMAND npx tsc --outDir "${BINARY_DIR}/dist"
)