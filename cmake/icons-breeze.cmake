include(ExternalProject)

function(download_breeze_icons VERSION)
    set(URL "https://github.com/KDE/breeze-icons/archive/refs/tags/v${VERSION}.zip")
    set(ZIP_FILE "${CMAKE_BINARY_DIR}/breeze-icons-${VERSION}.zip")
    set(EXTRACT_DIR "${CMAKE_BINARY_DIR}/breeze-icons-${VERSION}")
    set(breeze_icons_install_dir "${CMAKE_BINARY_DIR}/share/icons/breeze")

    
    file(DOWNLOAD "${URL}" "${ZIP_FILE}" SHOW_PROGRESS INACTIVITY_TIMEOUT 10 STATUS download_result)
    list(GET download_result 0 status_code)
    list(GET download_result 1 error_message)
    if (NOT status_code EQUAL 0)
        file(REMOVE "${path}")
        message(FATAL_ERROR "Failed to download ${URL}: ${error_message}")
    endif()

    file(ARCHIVE_EXTRACT INPUT "${ZIP_FILE}" DESTINATION "${CMAKE_BINARY_DIR}")
    file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/breeze-icons-${VERSION}/icons" NATIVE_FROM)
    file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/share/icons/breeze" NATIVE_TO)

    message(" *** dir /w ${NATIVE_FROM}")
    execute_process(
        COMMAND cmd /c "dir /w ${NATIVE_FROM}"
    )

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory "${NATIVE_TO}"
        RESULT_VARIABLE mkdir_result
    )
    if(mkdir_result)
        message(FATAL_ERROR "Error creating  ${NATIVE_TO}")
    endif()

    message(" *** dir /w ${NATIVE_TO}")
    execute_process(
        COMMAND cmd /c "dir /w ${NATIVE_TO}"
    )
    message(" *** Copying  ${NATIVE_FROM} ${NATIVE_TO} ")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_BINARY_DIR}/breeze-icons-${VERSION}/icons" "${CMAKE_BINARY_DIR}/share/icons/breeze"
        RESULT_VARIABLE copy_result
    )
    if(copy_result)
        message(FATAL_ERROR "Error copying ${NATIVE_FROM} => ${NATIVE_TO}")
    endif()
    
    set(${breeze_icons_install_dir_NATIVE} PARENT_SCOPE)
endfunction()
