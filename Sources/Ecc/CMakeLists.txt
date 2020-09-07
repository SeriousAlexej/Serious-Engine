set(TARGET_NAME Ecc)

set(SRC_LIST
    Main.cpp
    Main.h
    Parser.y
    Scanner.l
    StdH.h
)

set(PARSER_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/Parser.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/Parser.hxx
)

set(SCANNER_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/Scanner.cpp
)

add_executable(${TARGET_NAME}
    ${SRC_LIST}
    ${PARSER_SRC}
    ${SCANNER_SRC}
)

target_precompile_headers(${TARGET_NAME}
    PRIVATE StdH.h
)

add_custom_command(
    OUTPUT ${PARSER_SRC}
    COMMAND
        "${BISON_COMMAND}"
    ARGS
        "-o"
        "${CMAKE_CURRENT_BINARY_DIR}/Parser.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Parser.y"
        "-d"
    MAIN_DEPENDENCY Parser.y
)

add_custom_command(
    OUTPUT ${SCANNER_SRC}
    COMMAND
        "${FLEX_COMMAND}"
    ARGS
        "-o"
        "${CMAKE_CURRENT_BINARY_DIR}/Scanner.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Scanner.l"
    MAIN_DEPENDENCY Scanner.l
)
