file(GLOB_RECURSE TS_SRCS "${PKG_SRC_DIR}/*.ts")
ts_add_dependencies(${TS_SRCS})
ts_get_corresponding_js("${PKG_SRC_DIR}/main.ts" MAIN_JS)
# TODO: add a mechanism to verify the globbed files agains a hardcoded ones to
# make sure we don't miss any files.

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
    --alias:proto="${PROTO_JS_GEN_DIR}"
    --external:google-protobuf
    --external:grpc-web
    --external:three
    --outfile="${BUNDLE_JS}"
  DEPENDS
    ${TSCONFIG_JSON}
    ${PROTO_SRCS}
    proto_generate
    ${TS_SRCS}
    ts_compile
)

# TODO: This is actually not respecting transitively generated files. For
# example, if you change a .proto, then you need to trigger this target twice
# to get the updated bundle. This probably is an issue for other targets too.
add_custom_target(
  client ALL
  COMMENT "[client] ${BUNDLE_JS} up to date."
  DEPENDS ${BUNDLE_JS}
)