function (add_go_target TARGET_NAME)
	set(OPTIONS)
	set(ONE_VALUE_ARGS SRC_DIR BUILD_DIR SET_EXE)
	set(MULTI_VALUE_ARGS DEPENDS)
	cmake_parse_arguments(PARSE_ARGV 1 arg "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}")

	if (NOT arg_SRC_DIR)
		message(FATAL_ERROR "SRC_DIR is required")
	endif()
	if (NOT arg_BUILD_DIR)
		message(FATAL_ERROR "BUILD_DIR is required")
	endif()

	file(GLOB_RECURSE SRCS
		"${arg_SRC_DIR}/*.go"
		"${arg_SRC_DIR}/go.mod"
	)

	cmake_path(SET TARGET_EXE "${arg_BUILD_DIR}/${TARGET_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
	add_custom_command(
		COMMENT "[${TARGET_NAME}] Building ${TARGET_EXE}"
		OUTPUT ${TARGET_EXE}
		COMMAND go build -C "${arg_SRC_DIR}" -o "${TARGET_EXE}"
		DEPENDS ${SRCS} ${arg_DEPENDS}
	)

	add_custom_target(
		${TARGET_NAME} ALL
		COMMENT "[${TARGET_NAME}] ${TARGET_EXE} up to date."
		DEPENDS ${TARGET_EXE}
	)

	set(${arg_SET_EXE} ${TARGET_EXE} PARENT_SCOPE)
endfunction()
