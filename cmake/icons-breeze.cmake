# Include ExternalProject module
include(ExternalProject)

function(download_breeze_icons)
    message(STATUS "Downloading Breeze icons from archive...")
    set(BREEZE_ICONS_URL "https://github.com/KDE/breeze-icons/archive/refs/tags/v6.4.0.zip")
    set(DOWNLOAD_DIR "${CMAKE_BINARY_DIR}/breeze_icons_download")
    file(MAKE_DIRECTORY ${DOWNLOAD_DIR})
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/icons/breeze)
    ExternalProject_Add(
        breeze_icons_project
        PREFIX ${DOWNLOAD_DIR}
        URL ${BREEZE_ICONS_URL}
        DOWNLOAD_NAME v6.4.0.zip
        DOWNLOAD_DIR ${DOWNLOAD_DIR}
        CONFIGURE_COMMAND
            ${CMAKE_COMMAND} -E copy_directory ${DOWNLOAD_DIR}/src/breeze_icons_project/icons ${CMAKE_BINARY_DIR}/share/icons/breeze
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
        DOWNLOAD_EXTRACT_TIMESTAMP ON
    )
    message(STATUS "Breeze icons download and extraction completed")
endfunction()
