if(WIN32)
    set(BISON_COMMAND "${CMAKE_SOURCE_DIR}/../Tools.Win32/Bison.exe")
    set(FLEX_COMMAND  "${CMAKE_SOURCE_DIR}/../Tools.Win32/Flex.exe")
else()
    set(BISON_COMMAND "bison")
    set(FLEX_COMMAND  "flex")
endif()

############################################################################
function(bison_add TARGET_NAME PARSER_FILE)

    get_filename_component(PARSER_NAME ${PARSER_FILE} NAME_WE)
    get_filename_component(PARSER_DIR ${PARSER_FILE} DIRECTORY)
    if(PARSER_DIR)
        set(PARSER_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/${PARSER_DIR}")
        file(MAKE_DIRECTORY "${PARSER_BUILD_DIR}")
    else()
        set(PARSER_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    endif()
    
    set(BISON_ARGS
        "--output=${PARSER_BUILD_DIR}/${PARSER_NAME}.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/${PARSER_FILE}"
        "--defines=${PARSER_BUILD_DIR}/${PARSER_NAME}.h"
        "--name-prefix=${PARSER_NAME}_"
    )
    set(PARSER_SRC
        "${PARSER_BUILD_DIR}/${PARSER_NAME}.cpp"
        "${PARSER_BUILD_DIR}/${PARSER_NAME}.h"
    )
    add_custom_command(
        OUTPUT ${PARSER_SRC}
        COMMAND ${BISON_COMMAND}
        ARGS ${BISON_ARGS}
        MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/${PARSER_FILE}"
    )
    target_sources(${TARGET_NAME} PRIVATE ${PARSER_SRC})

    if(PARSER_DIR)
        string(REGEX REPLACE "/" "\\\\" PARSER_DIR "${PARSER_DIR}")
        source_group("Bison Autogen\\\\${PARSER_DIR}" FILES ${PARSER_SRC})
    else()
        source_group("Bison Autogen" FILES ${PARSER_SRC})
    endif()
endfunction(bison_add)

############################################################################
function(flex_add TARGET_NAME SCANNER_FILE)
  
    get_filename_component(SCANNER_NAME ${SCANNER_FILE} NAME_WE)
    get_filename_component(SCANNER_DIR ${SCANNER_FILE} DIRECTORY)
    if(SCANNER_DIR)
        set(SCANNER_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/${SCANNER_DIR}")
        file(MAKE_DIRECTORY "${SCANNER_BUILD_DIR}")
    else()
        set(SCANNER_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    endif()
    
    set(SCANNER_ARGS
        "--prefix=${SCANNER_NAME}_"
        "--nounistd"
        "--outfile=${SCANNER_BUILD_DIR}/${SCANNER_NAME}.cpp"
        "--header-file=${SCANNER_BUILD_DIR}/${SCANNER_NAME}.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/${SCANNER_FILE}"
    )
    
    set(SCANNER_SRC
        "${SCANNER_BUILD_DIR}/${SCANNER_NAME}.cpp"
        "${SCANNER_BUILD_DIR}/${SCANNER_NAME}.h"
    )
    add_custom_command(
        OUTPUT ${SCANNER_SRC}
        COMMAND ${FLEX_COMMAND}
        ARGS ${SCANNER_ARGS}
        MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/${SCANNER_FILE}"
    )
    target_sources(${TARGET_NAME} PRIVATE ${SCANNER_SRC})
    
    if(SCANNER_DIR)
        string(REGEX REPLACE "/" "\\\\" SCANNER_DIR "${SCANNER_DIR}")
        source_group("Flex Autogen\\\\${SCANNER_DIR}" FILES ${SCANNER_SRC})
    else()
        source_group("Flex Autogen" FILES ${SCANNER_SRC})
    endif()
endfunction(flex_add)
