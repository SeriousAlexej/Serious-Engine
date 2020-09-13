set(TARGET_NAME Ecc)

set(SRC_LIST
    Main.cpp
    Main.h
    Parser.y
    Scanner.l
    StdH.h
)

add_executable(${TARGET_NAME} ${SRC_LIST})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_LIST})

target_precompile_headers(${TARGET_NAME} PRIVATE StdH.h)

bison_add(${TARGET_NAME} Parser.y)
flex_add(${TARGET_NAME} Scanner.l)
