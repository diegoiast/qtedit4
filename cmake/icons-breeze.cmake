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
        BUILD_COMMAND ${CMAKE_COMMAND} -E copy_directory ${EXTRACT_DIR}/icons/actions/ ${breeze_icons_install_dir}/
                      ${CMAKE_COMMAND} -E copy_directory ${EXTRACT_DIR}/icons/mimetypes/ ${breeze_icons_install_dir}/
                      ${CMAKE_COMMAND} -E copy_directory ${EXTRACT_DIR}/icons/devices/ ${breeze_icons_install_dir}/
                      ${CMAKE_COMMAND} -E copy_directory ${EXTRACT_DIR}/icons/mimetypes/ ${breeze_icons_install_dir}/
                      ${CMAKE_COMMAND} -E copy ${EXTRACT_DIR}/icons/index.theme ${breeze_icons_install_dir}/
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
        DOWNLOAD_EXTRACT_TIMESTAMP ON
    )

    
#   # Use 7-Zip to extract the archive
#   add_custom_command(
#     OUTPUT ${EXTRACT_DIR}
#     COMMAND ${CMAKE_COMMAND} -E make_directory ${EXTRACT_DIR}
#     COMMAND "7z.exe" "x" "${ZIP_FILE}" "-o${EXTRACT_DIR}" "-aoa"
#     DEPENDS ${ZIP_FILE}
#     COMMENT "Extracting breeze-icons archive"
#     ERROR_VARIABLE extract_error
#     RESULT_VARIABLE extract_result
#   )
   
  set(${breeze_icons_install_dir} PARENT_SCOPE)
endfunction()
