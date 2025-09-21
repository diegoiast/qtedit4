# qtedit4 - an IDE for Rust, Go, C++ and more

![qtedit4](qtedit4.png)

![Build Status](https://github.com/diegoiast/qtedit4/actions/workflows/build.yml/badge.svg)
![Clang Format](https://github.com/diegoiast/qtedit4/actions/workflows/clang-format.yml/badge.svg)
![Code spell](https://github.com/diegoiast/qtedit4/actions/workflows/codespell.yml/badge.svg)
![Activity](https://img.shields.io/github/commit-activity/m/diegoiast/qtedit4)
![Open Issues](https://img.shields.io/github/issues-raw/diegoiast/qtedit4)

<!-- Community 
![Contributors](https://img.shields.io/github/contributors/diegoiast/qtedit4)
![Forks](https://img.shields.io/github/forks/diegoiast/qtedit4)
![Stars](https://img.shields.io/github/stars/diegoiast/qtedit4)
![Watchers](https://img.shields.io/github/watchers/diegoiast/qtedit4)
![Issues](https://img.shields.io/github/issues/diegoiast/qtedit4)
![Pull Requests](https://img.shields.io/github/issues-pr/diegoiast/qtedit4)
[![Buy Me a Coffee](https://img.shields.io/badge/-Buy%20Me%20a%20Coffee-FFDD00?style=flat&logo=buy-me-a-coffee&logoColor=black)](https://www.buymeacoffee.com/diegoiast)
![Closed Issues](https://img.shields.io/github/issues-closed/diegoiast/qtedit4)
![Open PRs](https://img.shields.io/github/issues-pr-raw/diegoiast/qtedit4)
![Closed PRs](https://img.shields.io/github/issues-pr-closed/diegoiast/qtedit4)
-->

qtedit4 is an IDE for Rust, Go, C++ Python and more. Its focus is local development, 
not web development. Its currently on early development. There are releases for Windows
(exe installer) and Linux (AppImage).

![Download latest stable version](https://img.shields.io/github/v/release/diegoiast/qtedit4)
 ![Download tetsing version](https://img.shields.io/github/v/release/diegoiast/qtedit4?include_prereleases)
![GitHub All Releases](https://img.shields.io/github/downloads/diegoiast/qtedit4/total)

Want to contribue? Look at [hack.md](hack.md) for guideance (TLDR: 
clone this repo, and `cmake -S build -B build; cmake --build build`, you will need a C++ compiler
, CMake and Qt installed). Keep commits readable, and rebase against main before
you open a pull-request

Code is released on the 1st of each month (usually a day, or two before), with an
RC/Beta/Alpha releases starting at the 20 of each month (sent to testing channels).

Kanban is avialble at https://github.com/users/diegoiast/projects/2
Feel free to open new issues, and work on whatever makes you happy.

## Status
 * Basic editor working. Supports only UTF8. (need other? [request for it](https://github.com/diegoiast/qtedit4/issues)
 * Can load/build projects: CMake, Meson, Go, Cargo
 * Preview for XML, JSON, SVG (!), Markdown
 * Binary/HEX editor
 * Simple image viewer
 * Auto update with stable and testing channels
 * Command pallete (contorl+P - open files, control+shift+P commands)
 * Auto save (modified files are restored on next app start)

Want to see it in action? Live coding videos here: https://www.youtube.com/playlist?list=PLQeAIMKe60pMtZp3KZTmYoYXySy6UlvD3

## License
Code is GPLv2 or higher. Read each file's as some files have a less-restrictive license. 

## History

The code was (once) hosted under [GoogleCode svn](https://code.google.com/archive/p/qtedit4/). Since,
Google [killed this project](https://killedbygoogle.com/) the code has been migrated to git, and is 
hosted in github. Each of the sub tools got its own git repo, (which was not trivial! but doable,
using `svndumpfilter` if you ask). Now, sub tools are loaded using CPM,
as child CMake projects.

Why `qtedit4`? When Qt3 was a thing QtDesginer had the ability to edit text, and was looking
like the idea was to make it a VisualBasic kind of IDE. Back then, the designer
had an internal text editor, I think the idea was to make QtCreator more like
VisualBasic. That was abandoned.

When Qt4 was released, my original code broke and I needed a soft reboot. I 
started a new project, called qtedit4. 

This means that some of the code originated in Qt3, and got migrated to Qt6 over the last
(too much) years.
