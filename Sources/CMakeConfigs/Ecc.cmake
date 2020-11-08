if(MSVC)
    set(ECC_EXECUTABLE "${CMAKE_BINARY_DIR}/RuntimeOutput/$<CONFIG>/Ecc")
else()
    set(ECC_EXECUTABLE "${CMAKE_BINARY_DIR}/RuntimeOutput/Ecc")
endif()

function(add_classes TARGET_NAME)
    add_dependencies(${TARGET_NAME} Ecc)
    target_include_directories(${TARGET_NAME} PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/..")
    foreach(CLASS_FILE IN ITEMS ${ARGN})
        get_filename_component(CLASS_NAME ${CLASS_FILE} NAME_WE)
        get_filename_component(CLASS_DIR ${CLASS_FILE} DIRECTORY)
        if(CLASS_DIR)
            set(CLASS_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/${CLASS_DIR}")
            file(MAKE_DIRECTORY "${CLASS_BUILD_DIR}")
        else()
            set(CLASS_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}")
        endif()

        set(CLASS_SOURCES
            "${CLASS_BUILD_DIR}/${CLASS_NAME}.cpp"
            "${CLASS_BUILD_DIR}/${CLASS_NAME}.h"
            "${CLASS_BUILD_DIR}/${CLASS_NAME}_tables.h"
        )

        add_custom_command(
            OUTPUT ${CLASS_SOURCES}
            COMMAND ${ECC_EXECUTABLE}
            ARGS "${TARGET_NAME}/${CLASS_FILE}" "${CLASS_BUILD_DIR}"
            MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/${CLASS_FILE}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/.."
        )

        target_sources(${TARGET_NAME} PRIVATE ${CLASS_SOURCES})

        if(CLASS_DIR)
            string(REGEX REPLACE "/" "\\\\" CLASS_DIR "${CLASS_DIR}")
            source_group("Ecc Autogen\\\\${CLASS_DIR}" FILES ${CLASS_SOURCES})
        else()
            source_group("Ecc Autogen" FILES ${CLASS_SOURCES})
        endif()
    endforeach()
endfunction(add_classes)
