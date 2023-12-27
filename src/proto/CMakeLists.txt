project(SpejsProto)

set(SRC_DIR ${PROJECT_SOURCE_DIR})
set(OUT_DIR ${PROJECT_BINARY_DIR})
set(CPP_OUT_DIR "${OUT_DIR}/cpp")
set(GO_OUT_DIR "${OUT_DIR}/go")

file(MAKE_DIRECTORY ${CPP_OUT_DIR})
file(MAKE_DIRECTORY ${GO_OUT_DIR})

set(GO_MOD "${GO_OUT_DIR}/go.mod")
add_custom_command(
  OUTPUT ${GO_MOD}
  COMMAND go mod init "github.com/mkacz91/spejs/pb"
  WORKING_DIRECTORY ${GO_OUT_DIR}
)

file(GLOB PROTO_SRCS "${SRC_DIR}/*.proto")
set(CPP_SRCS "")
set(GO_SRCS "")

foreach (FILE IN LISTS PROTO_SRCS)
  get_filename_component(STEM ${FILE} NAME_WE)
  file(RELATIVE_PATH RELATIVE_FILE ${SRC_DIR} ${FILE})
  set(CPP_BASE "${CPP_OUT_DIR}/${STEM}")
  set(CPP_OUT
    "${CPP_BASE}.pb.cc"
    "${CPP_BASE}.pb.h"
    "${CPP_BASE}.grpc.pb.cc"
    "${CPP_BASE}.grpc.pb.h"
  )
  set(GO_BASE "${GO_OUT_DIR}/${STEM}")
  set(GO_OUT
    "${GO_BASE}.pb.go"
    "${GO_BASE}_grpc.pb.go"
  )
  add_custom_command(
    OUTPUT ${CPP_OUT} ${GO_OUT}
    COMMAND ${PROTOC}
    --proto_path="${SRC_DIR}"
    --plugin=protoc-gen-grpc="${PROTOC_GRPC_CPP_PLUGIN}"
    --cpp_out="${CPP_OUT_DIR}"
    --grpc_out="${CPP_OUT_DIR}"
    --go_out=paths=source_relative:${GO_OUT_DIR}
    --go-grpc_out=paths=source_relative:${GO_OUT_DIR}
    "${FILE}"
    DEPENDS ${FILE}
  )
  list(APPEND CPP_SRCS ${CPP_OUT})
  list(APPEND GO_SRCS ${GO_OUT})
endforeach()

add_custom_target(generate_cpp_protos DEPENDS ${CPP_SRCS})
add_custom_target(generate_go_protos DEPENDS ${GO_SRCS} ${GO_MOD})

set(PROTO_CPP_OUT_DIR ${CPP_OUT_DIR} PARENT_SCOPE)
set(PROTO_CPP_SRCS ${CPP_SRCS} PARENT_SCOPE)
set(PROTO_GO_SRCS ${GO_SRCS} PARENT_SCOPE)
