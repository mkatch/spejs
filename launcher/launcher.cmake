file(GLOB_RECURSE LAUNCHER_SRCS
  "${PKG_SRC_DIR}/*.go"
  "${PKG_SRC_DIR}/go.mod"
)

cmake_path(SET LAUNCHER_EXE "${PKG_BUILD_DIR}/launcher${CMAKE_EXECUTABLE_SUFFIX}")
add_custom_command(
  COMMENT "[launcher] Building ${LAUNCHER_EXE}"
  OUTPUT ${LAUNCHER_EXE}
  COMMAND go build -C "${PKG_SRC_DIR}" -o "${LAUNCHER_EXE}"
  DEPENDS ${LAUNCHER_SRCS}
)

add_custom_target(
  launcher ALL
  COMMENT "[launcher] ${LAUNCHER_EXE} up to date."
  DEPENDS
    ${LAUNCHER_EXE}
    frontend_server
    # client
    universe_server
)

add_custom_target(
  spejs_run
  COMMENT "[launcher] Running ${LAUNCHER_EXE}"
  COMMAND ${LAUNCHER_EXE}
  WORKING_DIRECTORY ${ROOT_DIR}
  DEPENDS launcher
)