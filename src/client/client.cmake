file(GLOB_RECURSE TS_SRCS "${PKG_SRC_DIR}/*.ts")
cmake_path(SET MAIN_TS "${PKG_SRC_DIR}/main.ts")
list(REMOVE_ITEM TS_SRCS ${MAIN_TS})
# TODO: add a mechanism to verify the globbed files agains a hardcoded ones to
# make sure we don't miss any files.

extract_ts_files(MAIN_TS MAIN_JS)
extract_ts_files(TS_SRCS JS_SRCS)

cmake_path(SET BUNDLE_JS "${PKG_BUILD_DIR}/client.js")

add_custom_command(
  COMMENT "[client] Bundle into ${BUNDLE_JS}"
  OUTPUT ${BUNDLE_JS}
  COMMAND npx esbuild --bundle
    "${MAIN_JS}"
    --format=esm
    --tree-shaking=true
    --platform=browser
    --minify
    --external:google-protobuf
    --external:grpc-web
    --outfile="${BUNDLE_JS}"
  DEPENDS
    ${JS_SRCS}
    ${PROTO_JS_GEN_FILES}
)

add_custom_target(
  client ALL
  COMMENT "[client] ${BUNDLE_JS} up to date."
  DEPENDS ${BUNDLE_JS}
)