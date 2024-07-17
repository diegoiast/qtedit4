# Include ExternalProject module
include(ExternalProject)

include(ExternalProject)

function(download_breeze_icons VERSION)
    # Set URLs and directories
    set(URL "https://github.com/KDE/breeze-icons/archive/refs/tags/v${VERSION}.zip")
    set(ZIP_FILE "${CMAKE_BINARY_DIR}/breeze-icons-${VERSION}.zip")
    set(EXTRACT_DIR "${CMAKE_BINARY_DIR}/breeze-icons-${VERSION}")

    set(breeze_icons_install_dir "${CMAKE_BINARY_DIR}/share/icons/breeze")  # Define install directory
    ExternalProject_Add(
        breeze_icons_project
        URL ${URL}
        DOWNLOAD_NO_PROGRESS False
        DOWNLOAD_DIR ${CMAKE_BINARY_DIR}
        CONFIGURE_COMMAND
            ${CMAKE_COMMAND} -E copy_directory ${breeze_icons_project}/icons ${CMAKE_BINARY_DIR}/share/icons/
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
        DOWNLOAD_EXTRACT_TIMESTAMP ON
    )

    # Set the install directory as output of this function
    set(${breeze_icons_install_dir} PARENT_SCOPE)
endfunction()
