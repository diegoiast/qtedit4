include(ExternalProject)

function(download_breeze_icons VERSION)
    set(URL "https://github.com/KDE/breeze-icons/archive/refs/tags/v${VERSION}.zip")
    set(ZIP_FILE "${CMAKE_BINARY_DIR}/breeze-icons-${VERSION}.zip")
    set(EXTRACT_DIR "${CMAKE_BINARY_DIR}/breeze-icons-${VERSION}")

    set(breeze_icons_install_dir "${CMAKE_BINARY_DIR}/share/icons/breeze")
    ExternalProject_Add(
        breeze_icons_project
        URL ${URL}
        DOWNLOAD_NO_PROGRESS False
        DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
        SOURCE_DIR ${EXTRACT_DIR}
        CONFIGURE_COMMAND
            ${CMAKE_COMMAND} -E list_directory ${EXTRACT_DIR}
            ${CMAKE_COMMAND} -E list_directory ${EXTRACT_DIR}/icons
            ${CMAKE_COMMAND} -E list_directory ${breeze_icons_install_dir}
            ${CMAKE_COMMAND} -E copy_directory ${EXTRACT_DIR}/icons ${breeze_icons_install_dir}
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
        DOWNLOAD_EXTRACT_TIMESTAMP ON
    )

    set(${breeze_icons_install_dir} PARENT_SCOPE)
endfunction()
