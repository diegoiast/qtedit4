**Full Changelog**: https://github.com/diegoiast/qtedit4/compare/v0.0.13...v0.0.14-rc2

# October 2025 - release

A maintanence release, no big updates.

Selections are restored in opened files. Unsaved files are restored without saving (they
are actually saved to a temp location). You can modify the console fonts. Cargo bugs
are finally fixed. Copying a full document works as expected.

CMake dependencies are not locked with sha1, instead of following a branch. For reproducibility.

## Changelog
 * Meta: Save IDE state more periodically - https://github.com/diegoiast/qtedit4/issues/124
 * Completer: an empty line at the end - https://github.com/diegoiast/qutepart-cpp/issues/36
 * ProjectManager: target run is hard to find - https://github.com/diegoiast/qtedit4/issues/98
 * Text editor: Save content of unnamed files - https://github.com/diegoiast/qtedit4/issues/19
 * Indenter::indentBlock(): use Qt::key instead of QChar - https://github.com/diegoiast/qutepart-cpp/issues/45
 * File manager: right click on dir - add as project - https://github.com/diegoiast/qtedit4/issues/11
 * Project Manager: output should follow editor console - https://github.com/diegoiast/qtedit4/issues/111
 * Projectmanager: QProcess::openChannels: Inconsistent stderr channel configuration - https://github.com/diegoiast/qtedit4/issues/119
 * ProjectManager: cargo projects have no executables - https://github.com/diegoiast/qtedit4/issues/115
 * Editor: copying all lines does not add new line - https://github.com/diegoiast/qutepart-cpp/issues/47
 
