project(SpejsUniverse)

set(SRC_DIR ${PROJECT_SOURCE_DIR})

include_directories(
  ${GRPC_INCLUDE_DIR}
  ${PROTO_CPP_OUT_DIR}
)

file(GLOB UNIVERSE_SERVER_SRCS "${SRC_DIR}/*.cpp")
add_executable(universe_server ${UNIVERSE_SERVER_SRCS} ${PROTO_CPP_SRCS})
add_dependencies(universe_server generate_cpp_protos)
target_link_libraries(universe_server
  ${GRPC_LIBS}
)