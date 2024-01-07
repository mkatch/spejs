file(GLOB_RECURSE FRONTEND_SERVER_SRCS
  "${PKG_SRC_DIR}/*.go"
  "${PKG_SRC_DIR}/go.mod"
)

cmake_path(SET FRONTEND_SERVER_EXE "${PKG_BUILD_DIR}/frontend_server${CMAKE_EXECUTABLE_SUFFIX}")
add_custom_command(
  COMMENT "[frontend] Building ${FRONTEND_SERVER_EXE}"
  OUTPUT ${FRONTEND_SERVER_EXE}
  COMMAND go build -C "${PKG_SRC_DIR}" -o "${FRONTEND_SERVER_EXE}"
  DEPENDS ${FRONTEND_SERVER_SRCS}
)

add_custom_target(
  frontend_server ALL
  COMMENT "[frontend] ${FRONTEND_SERVER_EXE} up to date."
  DEPENDS ${FRONTEND_SERVER_EXE}
)

