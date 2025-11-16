**Full Changelog**: https://github.com/diegoiast/qtedit4/compare/v0.0.14...v0.0.15-rc1

# November 2025 - release

3 huge new features:

1. We gained the ability to execute scripts. Any file which is executable on Unix,
   or has an associated way for running (windows) will have a new icon (a small wheel) on the tab
   and its menu will contain a new command "Run Script". Pressing `control+shift+r` will execute
   this file. This should be used to run local python, shell or other types of scripts.
2. Folding support. The editor now supports folding areas. This is a work in progress with 
   the following known issues:
     - https://github.com/diegoiast/qutepart-cpp/issues/55
     - https://github.com/diegoiast/qutepart-cpp/issues/54
     - https://github.com/diegoiast/qutepart-cpp/issues/57
     
3. Smart file finding in the command pallete. The algorythm should be similar to whats seen in
   VScode or Sublime text (partial matching of file names, depending on path should work).
   See the following commits to understand this feature:
   https://github.com/diegoiast/command-palette-widget/commit/246cd7bce19eeabde1abc1f5eda1f8db955193d7
   https://github.com/diegoiast/command-palette-widget/commit/6289e63de4a3d83f286526b65351d5ee2033c71a

![qutepart folding](https://raw.githubusercontent.com/diegoiast/qutepart-cpp/main/qutepart.png)

## Changelog
* Project manager: run scripts - https://github.com/diegoiast/qtedit4/issues/5 
* Project manager: Open CMakLists.txt file - does not open a project - https://github.com/diegoiast/qtedit4/issues/122
* Qutepart: Add support for folding areas - https://github.com/diegoiast/qutepart-cpp/issues/19
* Command pallete: When searching commands - we should ignore accelerators  - https://github.com/diegoiast/command-palette-widget/issues/4

