function (add_proto_generate_target TARGET_NAME)
	set(OPTIONS)
	set(ONE_VALUE_ARGS SRC_DIR OUT_DIR GO_PACKAGE)
	set(MULTI_VALUE_ARGS LANGS)
	cmake_parse_arguments(PARSE_ARGV 1 arg "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}")

	if (NOT arg_SRC_DIR)
		message(FATAL_ERROR "SRC_DIR is required")
	endif()
	if (NOT arg_OUT_DIR)
		message(FATAL_ERROR "OUT_DIR is required")
	endif()

	file(GLOB SRCS "${arg_SRC_DIR}/*.proto")
	set(STAMPS "")

	set(CPP_SRCS "")
	set(CPP_OUT "")
	if ("cpp" IN_LIST arg_LANGS)
		cmake_path(SET CPP_OUT_DIR "${arg_OUT_DIR}")
		cmake_path(SET GRPC_OUT_DIR "${CPP_OUT_DIR}")
		set(CPP_ARGS 
			--cpp_out="${CPP_OUT_DIR}"
			--grpc_out="${GRPC_OUT_DIR}"
		)
	else()
		set(CPP_ARGS "")
	endif()

	if ("go" IN_LIST arg_LANGS)
		if (NOT arg_GO_PACKAGE)
			message(FATAL_ERROR "GO_PACKAGE is required for go language")
		endif()

		cmake_path(SET GO_OUT_DIR "${arg_OUT_DIR}/go")
		cmake_path(SET GO_GRPC_OUT_DIR "${GO_OUT_DIR}")

		cmake_path(SET GO_MOD "${GO_OUT_DIR}/go.mod")
		add_custom_command(
			COMMENT "[$TARGET_NAME] Initialize go module ${GO_MOD}"
			OUTPUT ${GO_MOD}
			COMMAND ${CMAKE_COMMAND} -E make_directory "${GO_OUT_DIR}"
			COMMAND ${CMAKE_COMMAND} -E
				chdir "${GO_OUT_DIR}"
				go mod init "${arg_GO_PACKAGE}"
		)

		set(GO_ARGS
      --plugin=protoc-gen-grpc="${PROTOC_GEN_GRPC_CPP_PLUGIN}"
			--go_out=paths=source_relative:"${GO_OUT_DIR}"
			--go-grpc_out=paths=source_relative:"${GO_GRPC_OUT_DIR}"
		)
	else()
		set(GO_ARGS "")
	endif()

	if ("js" IN_LIST arg_LANGS)
		cmake_path(SET JS_OUT_DIR "${arg_OUT_DIR}")
		cmake_path(SET GRPC_WEB_OUT_DIR "${JS_OUT_DIR}")
		make_directory(${JS_OUT_DIR})
		set(JS_ARGS
      --plugin=protoc-gen-js="${PROTOC_GEN_JS_PLUGIN}"
			--js_out=import_style=es6:"${JS_OUT_DIR}"
			--grpc-web_out=import_style=typescript,mode=grpcwebtext:"${GRPC_WEB_OUT_DIR}"
		)
	else()
		set(JS_ARGS "")
	endif()
	
	foreach (FILE ${SRCS})
		cmake_path(GET FILE STEM STEM)
		set(STAMP "${arg_OUT_DIR}/${STEM}.stamp")
		list(APPEND STAMPS ${STAMP})

		if ("cpp" IN_LIST arg_LANGS)
			# C++ generated files have to be explicitly added as target dependencies,
			# but for the other languages a .stamp is sufficient.
			set(CPP_OUT
				"${CPP_OUT_DIR}/${STEM}.pb.cc"
				"${CPP_OUT_DIR}/${STEM}.pb.h"
				"${GRPC_OUT_DIR}/${STEM}.grpc.pb.cc"
				"${GRPC_OUT_DIR}/${STEM}.grpc.pb.h"
			)
  		list(APPEND CPP_SRCS ${CPP_OUT})
		endif()

		add_custom_command(
			COMMENT "[${TARGET_NAME}] Running protoc on ${FILE}"
			OUTPUT ${CPP_OUT} ${STAMP}
			COMMAND ${PROTOC}
				--proto_path="${arg_SRC_DIR}"
				${CPP_ARGS}
				${GO_ARGS}
				${JS_ARGS}
				"${FILE}"
			COMMAND ${CMAKE_COMMAND} -E touch "${STAMP}"
			DEPENDS ${FILE}
		)
	endforeach()

	add_custom_target(
		${TARGET_NAME}
		DEPENDS ${STAMPS} ${GO_MOD}
		COMMENT "[${TARGET_NAME}] Protos generated."
	)

	if ("cpp" IN_LIST arg_LANGS)
		add_library("${TARGET_NAME}_cpp" ${CPP_SRCS})
	endif()
endfunction()


