file(GLOB PROTO_SRCS "${PKG_SRC_DIR}/*.proto")

cmake_path(SET CPP_OUT_DIR "${PKG_BUILD_DIR}")
cmake_path(SET GRPC_OUT_DIR "${CPP_OUT_DIR}")
cmake_path(SET GO_OUT_DIR "${PKG_BUILD_DIR}/go")
cmake_path(SET GO_GRPC_OUT_DIR "${GO_OUT_DIR}")
cmake_path(SET JS_OUT_DIR "${JS_BUILD_DIR}/${REL_PKG_BUILD_DIR}")
cmake_path(SET GRPC_WEB_OUT_DIR "${PKG_BUILD_DIR}")

set(STAMPS "")
set(CPP_GEN_FILES "")

foreach (FILE ${PROTO_SRCS})
  cmake_path(GET FILE STEM STEM)

  # C++ generated files have to be explicitly added as target dependencies, but
  # for the other languages a .stamp is sufficient.
  set(CPP_OUT
    "${CPP_OUT_DIR}/${STEM}.pb.cc"
    "${CPP_OUT_DIR}/${STEM}.pb.h"
    "${GRPC_OUT_DIR}/${STEM}.grpc.pb.cc"
    "${GRPC_OUT_DIR}/${STEM}.grpc.pb.h"
  )
  set(STAMP "${PKG_BUILD_DIR}/${STEM}.stamp")

  add_custom_command(
    COMMENT "[proto] Running protoc on ${FILE}"
    OUTPUT ${CPP_OUT} ${STAMP}
    COMMAND ${PROTOC}
      --proto_path="${PKG_SRC_DIR}"
      --plugin=protoc-gen-grpc="${PROTOC_GRPC_CPP_PLUGIN}"
      --cpp_out="${CPP_OUT_DIR}"
      --grpc_out="${GRPC_OUT_DIR}"
      --go_out=paths=source_relative:"${GO_OUT_DIR}"
      --go-grpc_out=paths=source_relative:"${GO_GRPC_OUT_DIR}"
      --js_out=import_style=commonjs:"${JS_OUT_DIR}"
      --grpc-web_out=import_style=typescript,mode=grpcwebtext:"${GRPC_WEB_OUT_DIR}"
      "${FILE}"
    COMMAND ${CMAKE_COMMAND} -E touch "${STAMP}"
    DEPENDS ${FILE}
  )

  list(APPEND STAMPS ${STAMP})
  list(APPEND CPP_GEN_FILES ${CPP_OUT})
endforeach()

cmake_path(SET GO_MOD "${GO_OUT_DIR}/go.mod")
add_custom_command(
  COMMENT "[proto] Initialize go module ${GO_MOD}"
  OUTPUT ${GO_MOD}
  COMMAND ${CMAKE_COMMAND} -E make_directory "${GO_OUT_DIR}"
  COMMAND ${CMAKE_COMMAND} -E
    chdir "${GO_OUT_DIR}"
    go mod init "github.com/mkacz91/spejs/pb"
)
list(APPEND GEN_FILES ${GO_MOD})

add_custom_target(
  proto_generate
  DEPENDS ${STAMPS}
  COMMENT "[proto] All protos generated."
)

ts_add_dependencies(${PROTO_SRCS} proto_generate)

set(PROTO_SRCS ${PROTO_SRCS} PARENT_SCOPE)
set(PROTO_CPP_GEN_FILES ${CPP_GEN_FILES} PARENT_SCOPE)
set(PROTO_JS_GEN_DIR ${JS_OUT_DIR} PARENT_SCOPE)