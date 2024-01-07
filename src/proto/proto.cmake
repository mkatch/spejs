file(GLOB PROTO_SRCS "${PKG_SRC_DIR}/*.proto")

cmake_path(SET PROTOC_CPP_OUT_DIR "${PKG_BUILD_DIR}")
cmake_path(SET PROTOC_GRPC_OUT_DIR "${PKG_BUILD_DIR}")
cmake_path(SET PROTOC_GO_OUT_DIR "${PKG_BUILD_DIR}/go")
cmake_path(SET PROTOC_GO_GRPC_OUT_DIR "${PROTOC_GO_OUT_DIR}")
cmake_path(SET PROTOC_JS_OUT_DIR "${JS_BUILD_DIR}/${REL_PKG_BUILD_DIR}")
cmake_path(SET PROTOC_GRPC_WEB_OUT_DIR "${PKG_BUILD_DIR}")

set(GEN_FILES "")

foreach (FILE ${PROTO_SRCS})
  cmake_path(GET FILE STEM STEM)
  snake_to_pascal_case(${STEM} PASCAL_STEM)

  set(PROTOC_OUT
    "${PROTOC_CPP_OUT_DIR}/${STEM}.pb.cc"
    "${PROTOC_CPP_OUT_DIR}/${STEM}.pb.h"
    "${PROTOC_GRPC_OUT_DIR}/${STEM}.grpc.pb.cc"
    "${PROTOC_GRPC_OUT_DIR}/${STEM}.grpc.pb.h"
    "${PROTOC_GO_OUT_DIR}/${STEM}.pb.go"
    "${PROTOC_GO_GRPC_OUT_DIR}/${STEM}_grpc.pb.go"
    "${PROTOC_JS_OUT_DIR}/${STEM}_pb.js"
    "${PROTOC_GRPC_WEB_OUT_DIR}/${STEM}_pb.d.ts"
    "${PROTOC_GRPC_WEB_OUT_DIR}/${PASCAL_STEM}ServiceClientPb.ts"
  )

  add_custom_command(
    COMMENT "[proto] Running protoc on ${FILE}"
    OUTPUT ${PROTOC_OUT}
    COMMAND ${PROTOC}
      --proto_path="${PKG_SRC_DIR}"
      --plugin=protoc-gen-grpc="${PROTOC_GRPC_CPP_PLUGIN}"
      --cpp_out="${PKG_BUILD_DIR}"
      --grpc_out="${PROTOC_GRPC_OUT_DIR}"
      --go_out=paths=source_relative:"${PROTOC_GO_OUT_DIR}"
      --go-grpc_out=paths=source_relative:"${PROTOC_GO_GRPC_OUT_DIR}"
      --js_out=import_style=commonjs:"${PROTOC_JS_OUT_DIR}"
      --grpc-web_out=import_style=typescript,mode=grpcwebtext:"${PROTOC_GRPC_WEB_OUT_DIR}"
      "${FILE}"
    DEPENDS ${FILE}
  )

  extract_ts_files(PROTOC_OUT TSC_OUT)
  list(APPEND GEN_FILES ${PROTOC_OUT} ${TSC_OUT})
endforeach()

cmake_path(SET GO_MOD "${PROTOC_GO_OUT_DIR}/go.mod")
add_custom_command(
  COMMENT "[proto] Initialize go module ${GO_MOD}"
  OUTPUT ${GO_MOD}
  COMMAND ${CMAKE_COMMAND} -E make_directory "${PROTOC_GO_OUT_DIR}"
  COMMAND ${CMAKE_COMMAND} -E
    chdir "${PROTOC_GO_OUT_DIR}"
    go mod init "github.com/mkacz91/spejs/pb"
)
list(APPEND GEN_FILES ${GO_MOD})

set(CPP_GEN_FILES ${GEN_FILES})
list(FILTER CPP_GEN_FILES INCLUDE REGEX ".*(\.cc|\.h)$")
set(PROTO_CPP_GEN_FILES ${CPP_GEN_FILES} PARENT_SCOPE)

set(JS_GEN_FILES ${GEN_FILES})
list(FILTER JS_GEN_FILES INCLUDE REGEX ".*(\.js|\.js.map)$")
set(PROTO_JS_GEN_FILES ${JS_GEN_FILES} PARENT_SCOPE)

add_custom_target(
  proto_generate
  DEPENDS ${GEN_FILES}
  COMMENT "[proto] All protos generated."
)