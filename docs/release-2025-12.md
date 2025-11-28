**Full Changelog**: https://github.com/diegoiast/qtedit4/compare/v0.0.15...v0.0.16-rc1

# December 2025 - release

A simple release for the end of the year. Mainly small bug fixes, and stability.
The only new feature is control+# (# is a number) will show/hide/select a docked
widget. Which is actually very helpful - if you press control+1/2/3 - you can hide
all docks and have a simple text editor.

## Changelog
* qmdieidtor: markdown preview does not work - https://github.com/diegoiast/qtedit4/issues/133
* qmdiServer: cannot abort closing all tabs - https://github.com/diegoiast/qmdilib/issues/27  
* qmdiEditor: better saving on close - https://github.com/diegoiast/qtedit4/issues/130
* Folding: minimap is not updated when folding changes - https://github.com/diegoiast/qutepart-cpp/issues/57
* Editor: display changes too much when switching editors - https://github.com/diegoiast/qutepart-cpp/issues/58
* PluginManager: shortcuts for showing docks - https://github.com/diegoiast/qmdilib/issues/28
* Editor: crash when closing adject tab - https://github.com/diegoiast/qtedit4/issues/128
* Minimap: current line is no longer updated - https://github.com/diegoiast/qutepart-cpp/issues/56
* Project manager: copy relative file to the project - https://github.com/diegoiast/qtedit4/issues/7
* When moving tabs to a new split, other tabs are lost sometimes - https://github.com/diegoiast/qtedit4/issues/131
* SplittabWidget: clicking on empty area creates a new file on the wrong split - https://github.com/diegoiast/qtedit4/issues/132


