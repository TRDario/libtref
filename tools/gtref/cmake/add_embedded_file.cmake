find_program(XXD xxd)

function(add_embedded_file TARGET FILE ARRAYNAME)
	set(FILEPATH "${FILE}")
	cmake_path(ABSOLUTE_PATH FILEPATH NORMALIZE)
	add_custom_command(
		OUTPUT ${FILEPATH}.hpp
		COMMAND ${XXD} -i -n ${ARRAYNAME} ${FILEPATH} ${FILEPATH}.hpp
		DEPENDS ${FILEPATH}
		COMMENT "Generating header ${FILE}.hpp"
		VERBATIM
	)
	target_sources(gtref PRIVATE ${FILEPATH}.hpp)
endfunction()