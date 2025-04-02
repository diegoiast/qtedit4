#pragma once

namespace GlobalCommands {
    // request to open a file
    inline constexpr const char* OpenFile = "OpenFile";
    
    // Broadcast - a new file has been loaded. The payload will contain the filename
    inline constexpr const char* LoadedFile = "LoadedFile";
    
    // Broadcast - build succeded.
    inline constexpr const char* BuildSucceeded = "BuildSucceeded";
    
    // Broadcast - a new project has been loaded (ctags plugin uses to load a tags file)
    inline constexpr const char* ProjectLoaded = "ProjectLoaded";

    // User clicked on an issue, scroll the build output to the line passed in payload
    inline constexpr const char* ScrollOutput = "ScrollOutput";

    // Editor requests information about variable bellow mouse
    inline constexpr const char* VariableInfo = "VariableInfo";

}


namespace GlobalArguments {
    inline constexpr const char* ProjectName = "ProjectName";
    inline constexpr const char* RequestedSymbol = "RequestedSymbol";
    inline constexpr const char* SourceDirectory = "SourceDirectory";
    inline constexpr const char* BuildDirectory = "BuildDirectory";
    inline constexpr const char* FileName = "FileName";
    inline constexpr const char* LineNumber = "LineNumber";
    inline constexpr const char* ColumnNumber = "ColumnNumber";
}