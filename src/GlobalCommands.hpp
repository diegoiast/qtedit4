#pragma once

#include <string>

namespace GlobalCommands {
    // request to open a file
    inline constexpr const char* OpenFile = "OpenFile";
    
    // Broadcast - a new file has been loaded. The payload will contain the filename
    inline constexpr const char* LoadedFile = "LoadedFile";
    
    // Broadcast - build succeded.
    inline constexpr const char* BuildSucceeded = "BuildSucceeded";
    
    // User clicked on an issue, scroll the build output to the line passed in payload
    inline constexpr const char* ScrollOutput = "ScrollOutput";
}
