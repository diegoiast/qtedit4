/**
 * \file GlobalCommands.hpp
 * \brief Definitions for commands constants
 * \author Diego Iastrubni diegoiast@gmail.com
 */

// SPDX-License-Identifier: MIT

#pragma once

namespace GlobalCommands {
// request to open a file
inline constexpr const char *OpenFile = "OpenFile";

// Broadcast - a new file has been loaded. The payload will contain the filename
inline constexpr const char *LoadedFile = "LoadedFile";

// Broadcast - build succeeded.
inline constexpr const char *BuildFinished = "BuildFinished";

// Broadcast - a new project has been loaded (ctags plugin uses to load a tags file)
inline constexpr const char *ProjectLoaded = "ProjectLoaded";

// Broadcast - a new project has been removed (ctags plugin uses to clear memory)
inline constexpr const char *ProjectRemoved = "ProjectRemoved";

// User clicked on an issue, scroll the build output to the line passed in payload
inline constexpr const char *ScrollOutput = "ScrollOutput";

// Editor requests information about variable below mouse
inline constexpr const char *VariableInfo = "VariableInfo";

// Editor requests information about variable below mouse
inline constexpr const char *KeywordTooltip = "KeywordTooltip";

} // namespace GlobalCommands

namespace GlobalArguments {
inline constexpr const char *ExactMatch = "ExactMatch";
inline constexpr const char *ProjectName = "ProjectName";
inline constexpr const char *RequestedSymbol = "RequestedSymbol";
inline constexpr const char *SourceDirectory = "SourceDirectory";
inline constexpr const char *BuildDirectory = "BuildDirectory";
inline constexpr const char *FileName = "FileName";
inline constexpr const char *Name = "Name";
inline constexpr const char *TaskName = "TaskName";
inline constexpr const char *LineNumber = "LineNumber";
inline constexpr const char *ColumnNumber = "ColumnNumber";
inline constexpr const char *Type = "Type";
inline constexpr const char *Value = "Value";
inline constexpr const char *Raw = "Raw";
inline constexpr const char *Tags = "Tags";
inline constexpr const char *Tooltip = "Tooltip";
inline constexpr const char *Symbol = "Symbol";
} // namespace GlobalArguments
