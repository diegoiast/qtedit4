
# https://copyprogramming.com/howto/embed-resources-eg-shader-code-images-into-executable-library-with-cmake

function(create_resources mask output)
    # Create empty output file
    file(WRITE ${CMAKE_BINARY_DIR}/${output} "")
    # Collect input files
    file(GLOB bins ${mask})
    # Iterate through input files
    foreach(bin ${bins})
        # Get short filename
        string(REGEX MATCH "([^/]+)$" filename ${bin})
        # Replace filename spaces & extension separator for C compatibility
        string(REGEX REPLACE "\\.| |-" "_" filename ${filename})
        # Read hex data from file
        file(READ ${bin} filedata HEX)
        # Convert hex data for C compatibility
        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})
        # Append data to output file
        file(APPEND ${CMAKE_BINARY_DIR}/${output} "const unsigned char ${filename}[] = {${filedata}};\nconst unsigned ${filename}_size = sizeof(${filename});\n")
    endforeach()
endfunction()

