# Tasks


## Project manager

1. ~~Support for chdir~~
1. ~~Implement delete build dir~~
1. ~~Shortcuts for run, build~~
1. ~~Task/Run submenus do nothing~~
1. ~~Stop task/executable~~
1. ~~Fix directories without json~~
1. ~~Remove project from IDE~~
1. ~~Save active project, target, command~~
1. ~~Add a watcher to the json, and reload on demand~~
1. Finish win32,win64,linux64,osx
1. ~~Auto deduct json for c++ dirs~~
1. ~~Auto deduct json for rust dirs~~
1. ~~Auto deduct json for go dirs~~
1. ~~Terminal adds extra lines (sometimes?)~~
1. ~~Automatic kit detection~~~
1. ~~Loading of large (real life) projects take too long (example: zed)~~
1. Sort order is wrong. It should sort by path and not string (visble again in zed)
1. Add GUI editor for json
1. Scripts files - run directly (in console, embeded)
1. Auto deduct json for Android dirs
1. ~~Plugins config loading should be done only when the main UI is up. ~~

## File manager plugin

1. Right click on a file should open a menu
    1. Basic file operations
    1. If is a file? Edit
    1. If is a dir? add as project

## QutePart

1. Pascal indentator
1. Fix crash in Kotlin indentator
1. Red line should be bellow text
1. Goto matching bracket
1. Current line number should be bold
1. Verify dark mode
1. Support for themes
1. Smart home/end
1. Load syntax files from disk
1. Download syntax files from kate.org

## qmdilib

1. ~~Save config in global space.~~
1. Shortcuts manager
1. Save restore cursor position in open windows
1. ~~Bottom view should be collapsible~~
1. All actions in menus, toolbars shold be "added" to the main window
1. Add actions to be forced at the end
1. API for config - each plugin should be able to define
   what options it needs (all via some json) and gui
   needs to be created for user to modify this json. (also - raw json
   access maybe?)

## Editor plugin

1. ~~Quick jump inside open files~~
1. File watcher on created file not stopped while saving
1. Save content of unnamed files
1. Add line/column to margin display
1. Copy path, full, relative
1. Make all actions availalbe from menus
1. Control+N should open a new tab.
1. Save zoom level per document
1. Line ending on save (Unix/Windows/Keep original)
1. Trim spaces on save (edit?)
1. F4 to switch header/definition

## Meta

1. MSI build
1. TGZ build for linux
1. OS build
1. Flatpack support
1. Linux: create desktop entry
1. Windows: verify that this can be used as global editor
1. Windows: Otter browser has a nice installer.
1. Restore window position properly


## Internal

1. ~~Name the main window~~
1. ~~Icon for main window~~
1. Hex editor support for binary files
1. Image viewer
1. Markdown display
1. Internal web browser? why not
1. First run actions
    1. Create kits
1. Project plugin will need a refactor, its getting too ugly.
1. SCM support
1. Global hotkeys https://github.com/Skycoder42/QHotkey/blob/master/README.md
1. Updater: https://github.com/alex-spataru/QSimpleUpdater, https://github.com/Skycoder42/QtAutoUpdater
1. Terminal widget: https://github.com/lxqt/qtermwidget (see what Kate did "back then".
1. QtAwesome icon set: https://github.com/gamecreature/QtAwesome
1. Breadcrumb: https://github.com/Qt-Widgets/wwwidgets-breadcrumb-color-combobox-led-spinbox-navigator-ip-etc
