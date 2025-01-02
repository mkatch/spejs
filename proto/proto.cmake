add_proto_generate_target(
  proto
  SRC_DIR "${PKG_SRC_DIR}"
  OUT_DIR "${PKG_BUILD_DIR}"
  LANGS "cpp" "go" "js"
  GO_PACKAGE "github.com/mkacz91/spejs/pb"
)