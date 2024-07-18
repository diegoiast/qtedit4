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
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ${CMAKE_COMMAND} -E make_directory ${breeze_icons_install_dir}
                      COMMAND ${CMAKE_COMMAND} -E copy_directory ${EXTRACT_DIR}/icons ${breeze_icons_install_dir}
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
        DOWNLOAD_EXTRACT_TIMESTAMP ON
    )

    # Print directory structure of EXTRACT_DIR before copying
    ExternalProject_Add_Step(
        breeze_icons_project print_extract_dir_structure
        COMMAND ${CMAKE_COMMAND} -E echo "Contents of ${EXTRACT_DIR}:"
        COMMAND ${CMAKE_COMMAND} -E list_directory ${EXTRACT_DIR}
        ALWAYS 1
        DEPENDEES download
    )

    # Print directory structure of breeze_icons_install_dir before copying
    ExternalProject_Add_Step(
        breeze_icons_project print_install_dir_structure
        COMMAND ${CMAKE_COMMAND} -E echo "Contents of ${breeze_icons_install_dir}:"
        COMMAND ${CMAKE_COMMAND} -E list_directory ${breeze_icons_install_dir}
        ALWAYS 1
        DEPENDEES print_extract_dir_structure
    )

    set(${breeze_icons_install_dir} ${breeze_icons_install_dir} PARENT_SCOPE)
endfunction()
