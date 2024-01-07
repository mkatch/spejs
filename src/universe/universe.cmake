file(GLOB UNIVERSE_SERVER_SRCS "${PKG_SRC_DIR}/*.cpp")
add_executable(universe_server
  ${UNIVERSE_SERVER_SRCS}
  ${PROTO_CPP_GEN_FILES}
)
target_link_libraries(universe_server
  ${GRPC_LIBS}
)