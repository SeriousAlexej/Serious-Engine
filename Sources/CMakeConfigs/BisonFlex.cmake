if(WIN32)
    set(BISON_COMMAND "${CMAKE_SOURCE_DIR}/../Tools.Win32/Bison.exe")
    set(FLEX_COMMAND  "${CMAKE_SOURCE_DIR}/../Tools.Win32/Flex.exe")
else()
    set(BISON_COMMAND "bison")
    set(FLEX_COMMAND  "flex")
endif()

############################################################################
function(bison_add TARGET_NAME PARSER_FILE)

    get_filename_component(PARSER_NAME ${CMAKE_CURRENT_SOURCE_DIR}/${PARSER_FILE} NAME_WE)
    
    set(BISON_ARGS
        "--output=${CMAKE_CURRENT_BINARY_DIR}/${PARSER_NAME}.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/${PARSER_FILE}"
        "--defines=${CMAKE_CURRENT_BINARY_DIR}/${PARSER_NAME}.h"
        "--name-prefix=${PARSER_NAME}_"
    )
    set(PARSER_SRC
        ${CMAKE_CURRENT_BINARY_DIR}/${PARSER_NAME}.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/${PARSER_NAME}.h
    )
    add_custom_command(
        OUTPUT ${PARSER_SRC}
        COMMAND "${BISON_COMMAND}"
        ARGS ${BISON_ARGS}
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${PARSER_FILE}
    )
    target_sources(${TARGET_NAME} PRIVATE ${PARSER_SRC})
    
    source_group("Bison Autogen" FILES ${PARSER_SRC})
    
endfunction(bison_add)

############################################################################
function(flex_add TARGET_NAME SCANNER_FILE)

    cmake_parse_arguments(ARG "" "" "SKELETON_FILE" ${ARGN})
    
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unrecognized arguments: '${ARG_UNPARSED_ARGUMENTS}'")
    endif()
  
    get_filename_component(SCANNER_NAME ${CMAKE_CURRENT_SOURCE_DIR}/${SCANNER_FILE} NAME_WE)
    
    set(SCANNER_ARGS
        "--prefix=${SCANNER_NAME}_"
        "--nounistd"
        "--outfile=${CMAKE_CURRENT_BINARY_DIR}/${SCANNER_NAME}.cpp"
        "--header-file=${CMAKE_CURRENT_BINARY_DIR}/${SCANNER_NAME}.h"
    )
    
    if(ARG_SKELETON_FILE)
        list(APPEND SCANNER_ARGS "--skel=${ARG_SKELETON_FILE}")
    endif()
    
    list(APPEND SCANNER_ARGS "${CMAKE_CURRENT_SOURCE_DIR}/${SCANNER_FILE}")
    
    set(SCANNER_SRC
        ${CMAKE_CURRENT_BINARY_DIR}/${SCANNER_NAME}.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/${SCANNER_NAME}.h
    )
    add_custom_command(
        OUTPUT ${SCANNER_SRC}
        COMMAND ${FLEX_COMMAND}
        ARGS ${SCANNER_ARGS}
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${SCANNER_FILE}
    )
    target_sources(${TARGET_NAME} PRIVATE ${SCANNER_SRC})
    
    source_group("Flex Autogen" FILES ${SCANNER_SRC})
    
endfunction(flex_add)
