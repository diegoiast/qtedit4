**Full Changelog**: https://github.com/diegoiast/qtedit4/compare/v0.0.12...v0.0.13

# September 2025 - release

This release fixes hihgilighter issues introduced in previous
releases, as well as some optimizations to the editor. It feels in par with what
QtCreator provides. Loading files will probably not become any faster. Indenting
selected text works now.

Building Rust/cargo projects is improved, the build logs are properly parsed now.
The project parsing is still not trivial (waiting for a solution to https://github.com/diegoiast/qtedit4/issues/115)
but projects are still buildable, and you can still run from the UI `cargo run`,
assuming its set up properly in your cargo project.

Plugins have now async API, which I will use eventually on the LSP plugin.
Deletion of json project files will no longer kill the app. Split tabs now
can be moved with keyboard, and the close button works reliably.

Installing the program on a clean Windows 10/11 will trigger a request to install
vcredist, as this is a hidden dependency of Qt.

## Changelog

 * Highlighter: comment block on existing text fails - https://github.com/diegoiast/qutepart-cpp/issues/41
 * Projectmanager: running executable is not always working - https://github.com/diegoiast/qtedit4/issues/90
 * ProjectManager/rust: errors shows, clicking on file is wrong - https://github.com/diegoiast/qtedit4/issues/116
 * PluginManager: convert the API for getting completions to async - https://github.com/diegoiast/qtedit4/issues/114
 * editor: indenter for a selected block does not work  - https://github.com/diegoiast/qutepart-cpp/issues/44
 * Intenter: cursor at wrong position after indent - https://github.com/diegoiast/qutepart-cpp/issues/44
 * Intenter: cursor at wrong position after indent - https://github.com/diegoiast/qutepart-cpp/issues/42
 * SplitTabWidget: move current tab with keyboard - https://github.com/diegoiast/qtedit4/issues/113
 * SplitTabWidget: close button closes wrong tab - https://github.com/diegoiast/qtedit4/issues/117
 * meta: remove usage of foreach - https://github.com/diegoiast/qmdilib/issues/26
 * ProjectManager: save not working in minimal mode - https://github.com/diegoiast/qmdilib/issues/25
 * Double click will edit the item  - https://github.com/diegoiast/command-palette-widget/issues/3
 * Editor: hide the toolbar - https://github.com/diegoiast/qtedit4/issues/118
 * Meta: fail to run on clean install of windows 11 - https://github.com/diegoiast/qtedit4/issues/120
