function (add_proto_generate_target TARGET_NAME)
	set(OPTIONS)
	set(ONE_VALUE_ARGS SRC_DIR GO_PACKAGE)
	set(MULTI_VALUE_ARGS LANGS)
	cmake_parse_arguments(PARSE_ARGV 1 arg "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}")

	if (NOT arg_SRC_DIR)
		message(FATAL_ERROR "SRC_DIR is required")
	endif()
	get_relative_path_or_die("${arg_SRC_DIR}" REL_SRC_DIR)

	file(GLOB SRCS "${arg_SRC_DIR}/*.proto")
	set(TARGET_DEPS "")
	set(PROTOC_ARGS "")

	set(CPP_SRCS "")
	cmake_path(SET CPP_OUT_DIR "${CPP_GEN_DIR}/${REL_SRC_DIR}" )
	if ("cpp" IN_LIST arg_LANGS)
		cmake_path(SET CPP_OUT_DIR "${CPP_GEN_DIR}/${REL_SRC_DIR}")
		list(APPEND PROTOC_ARGS 
			--cpp_out="${CPP_GEN_DIR}"
			--grpc_out="${CPP_GEN_DIR}"
		)
	endif()

	set(GO_SRCS "")
	if ("go" IN_LIST arg_LANGS)
		if (NOT arg_GO_PACKAGE)
			message(FATAL_ERROR "GO_PACKAGE is required for go language")
		endif()
		cmake_path(SET GO_MOD_DIR "${GO_GEN_DIR}/${REL_SRC_DIR}")
		cmake_path(SET GO_MOD "${GO_MOD_DIR}/go.mod")
		add_custom_command(
			COMMENT "[${TARGET_NAME}] Initialize go module ${GO_MOD}"
			OUTPUT ${GO_MOD}
			COMMAND ${CMAKE_COMMAND} -E make_directory "${GO_MOD_DIR}"
			COMMAND ${CMAKE_COMMAND} -E chdir "${GO_MOD_DIR}" go mod init "${arg_GO_PACKAGE}"
		)
		list(APPEND TARGET_DEPS ${GO_MOD})
		list(APPEND PROTOC_ARGS
      --plugin=protoc-gen-grpc="${PROTOC_GEN_GRPC_CPP_PLUGIN}"
			--go_out=paths=source_relative:"${GO_GEN_DIR}"
			--go-grpc_out=paths=source_relative:"${GO_GEN_DIR}"
		)
	endif()
	
	cmake_path(SET JS_OUT_DIR "${JS_GEN_DIR}/${REL_SRC_DIR}")
	cmake_path(SET JS_INTERNAL_OUT_DIR "${JS_INTERNAL_GEN_DIR}/${REL_SRC_DIR}")
	if ("js" IN_LIST arg_LANGS)
		list(APPEND PROTOC_ARGS
      --plugin=protoc-gen-js="${PROTOC_GEN_JS_PLUGIN}"
			--js_out=import_style=browser:"${JS_INTERNAL_GEN_DIR}"
			--grpc-web_out=import_style=typescript,mode=grpcwebtext:"${JS_INTERNAL_GEN_DIR}"
		)
	endif()
	
	foreach (FILE ${SRCS})
		cmake_path(GET FILE STEM STEM)
		set(OUTPUT "")

		if ("cpp" IN_LIST arg_LANGS)
			cmake_path(SET PB_CPP "${CPP_OUT_DIR}/${STEM}.pb.cc")
			cmake_path(SET PB_H "${CPP_OUT_DIR}/${STEM}.pb.h")
			cmake_path(SET GRPC_PB_CPP "${CPP_OUT_DIR}/${STEM}.grpc.pb.cc")
			cmake_path(SET GRPC_PB_H "${CPP_OUT_DIR}/${STEM}.grpc.pb.h")
			set(CPP_OUTPUT ${PB_CPP} ${PB_H} ${GRPC_PB_CPP} ${GRPC_PB_H})
			list(APPEND OUTPUT ${CPP_OUTPUT})
  		list(APPEND CPP_SRCS ${CPP_OUTPUT})
		endif()

		if ("go" IN_LIST arg_LANGS)
			cmake_path(SET GO_PB "${GO_GEN_DIR}/${REL_SRC_DIR}/${STEM}.pb.go")
			cmake_path(SET GO_GRPC_PB "${GO_GEN_DIR}/${REL_SRC_DIR}/${STEM}_grpc.pb.go")
			set(GO_OUTPUT ${GO_PB} ${GO_GRPC_PB})
			list(APPEND GO_SRCS ${GO_OUTPUT})
			list(APPEND OUTPUT ${GO_OUTPUT})
		endif()

		if ("js" IN_LIST arg_LANGS)
			cmake_path(SET TS_OUTPUT "${JS_OUT_DIR}/${STEM}.ts")
			list(APPEND JS_SRCS ${TS_OUTPUT})
			list(APPEND OUTPUT ${TS_OUTPUT})
			set(FIXUP_JS_INPUT "${FILE}")
		else()
			set(FIXUP_JS_INPUT "")
		endif()

		list(APPEND TARGET_DEPS ${OUTPUT})

		add_custom_command(
			COMMENT "[${TARGET_NAME}] Generating protos from ${FILE}"
			OUTPUT ${OUTPUT}
			COMMAND ${PROTOC}
				--proto_path="${ROOT_DIR}"
				${PROTOC_ARGS}
				"${FILE}"
			COMMAND ${SPEJS_CLI} fixup-js-proto
				--input="${FIXUP_JS_INPUT}"
				--internal-dir="${JS_INTERNAL_OUT_DIR}"
				--out-dir="${JS_OUT_DIR}"
			DEPENDS ${FILE} ${SPEJS_CLI}
		)
	endforeach()

	add_custom_target(
		${TARGET_NAME}
		DEPENDS ${TARGET_DEPS}
		COMMENT "[${TARGET_NAME}] Protos generated."
	)

	if ("cpp" IN_LIST arg_LANGS)
		add_library("${TARGET_NAME}_cpp" ${CPP_SRCS})
	endif()
endfunction()
