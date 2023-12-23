project(SpejsUniverse)

set(SRC_DIR ${PROJECT_SOURCE_DIR})
set(PROTO_GENERATED_DIR "${PROJECT_BINARY_DIR}/proto")

file(MAKE_DIRECTORY ${PROTO_GENERATED_DIR})

include_directories(
  ${GRPC_INCLUDE_DIR}
  ${PROTO_GENERATED_DIR}
)

set(PROTO_GENERATED_FILES "")
foreach (FILE IN LISTS PROTO_SRCS)
  get_filename_component(STEM ${FILE} NAME_WE)
  set(GENERATED_BASE "${PROTO_GENERATED_DIR}/${STEM}")
  set(GENERATED_FILES
    "${GENERATED_BASE}.pb.cc"
    "${GENERATED_BASE}.pb.h"
    "${GENERATED_BASE}.grpc.pb.cc"
    "${GENERATED_BASE}.grpc.pb.h"
  )
  add_custom_command(
    OUTPUT ${GENERATED_FILES}
    COMMAND ${PROTOC}
    --plugin=protoc-gen-grpc="${PROTOC_GRPC_CPP_PLUGIN}"
    --cpp_out="${PROTO_GENERATED_DIR}"
    --grpc_out="${PROTO_GENERATED_DIR}"
    --proto_path="${PROTO_SRC_DIR}"
    "${FILE}"
    DEPENDS ${FILE}
  )
  list(APPEND PROTO_GENERATED_FILES ${GENERATED_FILES})
endforeach()

file(GLOB UNIVERSE_SERVER_SRCS "${SRC_DIR}/*.cpp")
add_executable(universe_server ${UNIVERSE_SERVER_SRCS} ${PROTO_GENERATED_FILES})
target_link_libraries(universe_server
  ${GRPC_LIBS}
  
)