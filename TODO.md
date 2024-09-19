# Tasks


Tasks for v0.0.1

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
1. ~~Automatic kit detection~~
1. ~~Loading of large (real life) projects take too long (example: zed)~~
1. ~~Plugins config loading should be done only when the main UI is up.~~
1. ~~Kits without Qt~~
1. Sort order is wrong. It should sort by path and not string (visble again in zed)
1. Detect build errors/warnings
1. Add GUI editor for json
1. Scripts files - run directly (in console, embeded)
1. Auto deduct json for Android dirs

## File manager plugin

1. Right click on a file should open a menu
    1. ~~Basic file operations (rename, delete, copy, paste)~~
    1. ~~If is a file? Edit~~
    1. If is a dir? add as project
    1. New empty file
    1. New dir
    1. Multiple selection
    1. Copy multiple files
    1. Delete multiple files
    1. Cut is not implemented
    1. Copy file name (relative, full, etc)

## QutePart

1. ~~Red line should be bellow text: made semi transpatent~~
1. ~~Current line number should be bold~~
1. ~~Verify dark mode~~
1. ~~Smart home/end + enable/disable~~
1. Goto matching bracket
1. Fix crash in Kotlin indentator
1. Support for themes
1. Load syntax files from disk
1. Download syntax files from kate.org
1. Pascal indentator
1. Select all words in document (if you put the cursor over a word
   all instances should be selected, using extraSelections).

## qmdilib

1. ~~Save config in global space.~~
1. ~~Save restore cursor position in open windows~~
1. ~~Bottom view should be collapsible~~
1. ~~All actions in menus, toolbars shold be "added" to the main window: was a bug~~
1. ~~API for config - each plugin should be able to define~~
   ~~what options it needs (all via some json) and gui~~
   ~~needs to be created for user to modify this json. (also - raw json~~
   ~~access maybe?)~~
1. ~~Saving does not query open files, should stall and ask for saving.~~
1. Shortcuts manager
1. Fix plugin manager demo with new config API, and shortcuts manager
1. Add actions to be forced at the end

## Editor plugin

1. ~~Quick jump inside open files~~
1. ~~Save zoom level per document~~
1. ~~File watcher on created file not stopped while saving~~
1. Control+N should open a new tab.
1. ~~Line ending on save (Unix/Windows/Keep original)~~~
1. ~~Trim spaces on save (edit?)~~
1. ~~Add line/column to margin display~~
1. ~~Switch indentator/language~~
1. ~~When saving/loading a file - and it not recognized, the UI~~
   ~~should reflect it somehow. See `updateFileDetails()`.~~
1. ~~When loading a file - if it seems binary, load with another plugin~~
  ~~(hardocde to hexedit?)~~
1. ~~Copy path, full~~
1. Make all actions availalbe from menus
1. F4 to switch header/definition
1. Defer loading 50msec after creation (will improve firs time load)
1. Need to find a way to keep original line ending
1. Save content of unnamed files
1. Copy path relative to project

## Meta

1. Restore window position properly
1. MSI build
1. TGZ build for linux
1. OSX build
1. Flatpack support
1. Linux: create desktop entry
1. Windows: verify that this can be used as global editor
1. Windows: Otter browser has a nice installer.


## Internal

1. ~~Name the main window~~
1. ~~Icon for main window~~
1. ~~Hex editor support for binary files, https://github.com/Dax89/QHexView~~
1. ~~Image viewer (used palacaze/image-viewer)~~
1. Markdown display
1. First run actions
    1. Create kits
    1. Windows:
        1. Download clang
        1. Download cmake
        1. Download ninja
        1. Download git
        1. Download Qt (?)
        1. SCM support
1. Project plugin will need a refactor, its getting too ugly.
1. Internal web browser? why not
1. Global hotkeys https://github.com/Skycoder42/QHotkey/blob/master/README.md
1. Updater: https://github.com/alex-spataru/QSimpleUpdater, https://github.com/Skycoder42/QtAutoUpdater
1. Terminal widget: https://github.com/lxqt/qtermwidget (see what Kate did "back then")
1. QtAwesome icon set: https://github.com/gamecreature/QtAwesome
1. Breadcrumb: https://github.com/Qt-Widgets/wwwidgets-breadcrumb-color-combobox-led-spinbox-navigator-ip-etc
1. Lots: https://timschneeberger.me/qt/
    1. https://github.com/timschneeb/FlatTabWidget
    1. https://github.com/timschneeb/QAnimatedSlider
1. Lots: https://github.com/progzdeveloper/QtExtra
1. light? dark? https://github.com/Qt-Widgets/ThemeSupport-determine-OS-currently-active-theme
1. Flutent design: https://github.com/githubuser0xFFFF/QtFluentDesign
1. Modern style: https://github.com/Qt-Widgets/Lightly_modern_style_theme
1. Adwita for QT: https://github.com/Qt-Widgets/adwaita-qt-style-qstyle
1. QJsonModel: https://github.com/dridk/QJsonModel
