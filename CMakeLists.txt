cmake_minimum_required(VERSION 3.28.1 FATAL_ERROR)
project(Spejs)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_CXX_STANDARD 20)

cmake_path(SET ROOT_DIR ${CMAKE_SOURCE_DIR})
cmake_path(SET CMAKE_MODULE_PATH "${ROOT_DIR}/cmake")
cmake_path(SET TSCONFIG_JSON "${ROOT_DIR}/tsconfig.json")

cmake_path(SET BUILD_DIR ${CMAKE_BINARY_DIR})
cmake_path(RELATIVE_PATH BUILD_DIR OUTPUT_VARIABLE REL_BUILD_DIR)
if (NOT REL_BUILD_DIR STREQUAL "build")
  message(FATAL_ERROR
    "Build directory must be named 'build' "
    "and be located in the root directory of the project."
  )
endif()

find_package(GLM REQUIRED)
find_package(Glad REQUIRED)
find_package(GLFW REQUIRED)
find_package(gRPC REQUIRED)
find_package(OpenGL REQUIRED)
find_package(QOI REQUIRED)

cmake_path(SET CPP_GEN_DIR "${BUILD_DIR}/gen_cpp")
make_directory(${CPP_GEN_DIR})
cmake_path(SET JS_GEN_DIR "${BUILD_DIR}/gen_js")
make_directory(${JS_GEN_DIR})
cmake_path(SET JS_INTERNAL_GEN_DIR "${BUILD_DIR}/gen_js_internal")
make_directory(${JS_INTERNAL_GEN_DIR})
cmake_path(SET GO_GEN_DIR "${BUILD_DIR}/gen_go")
make_directory(${GO_GEN_DIR})

include_directories(
  ${GLM_INCLUDE_DIR}
  ${GLFW_INCLUDE_DIR}
  ${GRPC_INCLUDE_DIR}
  ${QOI_INCLUDE_DIR}
  ${ROOT_DIR}
  ${CPP_GEN_DIR}
)

include(cmake/util.cmake)
include(cmake/proto.cmake)
include(cmake/go.cmake)

function(include_pkg REL_PKG_CMAKE)
  cmake_path(SET PKG_CMAKE "${ROOT_DIR}/${REL_PKG_CMAKE}")
  cmake_path(GET REL_PKG_CMAKE PARENT_PATH REL_PKG_DIR)
  cmake_path(SET PKG_SRC_DIR "${ROOT_DIR}/${REL_PKG_DIR}")
  cmake_path(GET PKG_CMAKE STEM PKG_NAME)
  cmake_path(SET PKG_BUILD_DIR "${BUILD_DIR}/${REL_PKG_DIR}")
  cmake_path(SET REL_PKG_BUILD_DIR "${REL_BUILD_DIR}/${REL_PKG_DIR}")
  file(MAKE_DIRECTORY ${PKG_BUILD_DIR})

  include(${PKG_CMAKE})
endfunction()

include_pkg(cli/cli.cmake)
include_pkg(proto/proto.cmake)
include_pkg(gl_cpp/gl_cpp.cmake)
include_pkg(shader_bundler/shader_bundler.cmake)
include_pkg(client/client.cmake)
include_pkg(frontend/frontend.cmake)
include_pkg(universe/universe.cmake)
include_pkg(launcher/launcher.cmake)
