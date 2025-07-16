# Hacking on QtEdit4

So you want to hack on QtEdit4. You have chosen poorly. Here
are some of the reasons why you should not be using C++ for such 
project:


1. C++ is a bad language. 
   1. Initializing variables in a pain, see http://mikelui.io/2019/01/03/seriously-bonkers.html
   1. No package manager. We use CPM, which has no transient dependencies: https://github.com/cpm-cmake/CPM.cmake
   1. The ABI issue is a huge pain, which currently we avoid by not having
      external plugins. Yet.
   1. Apparently its unsafe: https://www.cisa.gov/resources-tools/resources/product-security-bad-practices
   1. Basic things like recommended indentation, or naming convention does
      not exist like in more modern lanaguages like GoLang or Rust.
      Some of those things can be fixed by using a `.clang-format`,
      and enforcing this at the merge level.
   1. There is no proper way to distribute applications in Linux.
      While flatpack exists - it is not ideal. Instead, this project
      uses [AppImage](https://appimage.org/), which is a complete mess and contains no good
      documentation.
1. We are using Qt6 for the UI. Which is not ideal
   1. Lots of work is done in the main thread. You want to 
      store 100,000 items in a list view? Sure, this is done in the
      main thread. If this also a filtered sort filter, this filtering
      is don in the main thread as well.
   1. `QPlainTextEdit` syntax highlighting is done in a single
       pass, not incrementally (like GEdit or Scintilla does), which
       means loading a large file, might block the main UI.
   1. There is no concept of "default icons" in Qt. Whilte Qt 6.8
      starts naming the icons in a "theme", it actually only works
      on Gnu/Linux distros. On Windows, you need to deploy your own.
      This project bundles its own "breeze" icons from KDE, see 
      [icons-breeze.cmake](https://github.com/diegoiast/qtedit4/blob/main/cmake/icons-breeze.cmake)
   1. While Qt has an abstration for "settings", its not enough.
      1. There is no way to enforce a "schema".
      1. There is no "good" way to modify a menu/toolbar. Problems
         like "inject" a new submenu, or actions into a toolbar, once created.
         This project uses [qmdilib](https://github.com/diegoiast/qmdilib/).
      1. There is no way to create a UI for editing the config,
         leaving programmer to manually create UI. Unlike [android](https://developer.android.com/develop/ui/views/components/settings)
         or [IOS](https://developer.apple.com/documentation/foundation/userdefaults).
         This project uses qmdilib - see [demo3 in this library for our implementation/(https://github.com/diegoiast/qmdilib/blob/main/demos/demo3/main3.cpp). 
    1. There is no native editor control. So once needs to be created. This project
         uses [QutePart](https://github.com/diegoiast/qutepart-cpp) which is 
         slow and lacks features.
3. Building an IDE is one of the most challenging projects 

Not to mention that writing good text editor, or an IDE is extreamly
hard. This is a fight we are doing up-hill. 

But you already know this, this is the reason you are here.

## Building on Windows

To build on Windows, you will need to install:

1. [Qt 6.8.1](https://www.qt.io/download-open-source). You can use the Open Source
   edition.
2. [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/). 
   You can use the Community edition. *Optional step*.
3. [CMake](https://cmake.org/download/). Any version above 3.24 is good for us.
   *Optional step*.
4. [GitHubDesktop](https://desktop.github.com/download/). I find Github desktop 
   the best/simplest git experience on Windows. You can use also 
   [Git for window](https://git-scm.com/downloads/win) but it 
   requires more manual handling.
5. [InnoSetup](https://jrsoftware.org/isinfo.php). Required only if you want
   to create installers.

**NOTE**
> The Qt installation comes with a CMake build, and a MinwGW compiler. Thoses
> components can used for building.
>
> You can optionally use the standalone/original CMake setup, or use MSVC 
> to build. That is what the CD/CI does. I test both scenarions regularly.
>
> The command line git utilites also work perfectly.

Steps:

1. Using Github desktop - clone the repo.
1. Open QtCreator, and choose the `CMakeLists.txt` of this project.
    1. Press `Control+R`, and wait 5-10 minutes (initial configuration will
       download lots of source, and it will build it locally).
    1. If things do not compile, it generally means a Kits problem.
    See https://doc.qt.io/qtcreator/creator-preferences-kits.html
1. If you use Visual Studio, choose from the main window "Open a local folder".
    1. Visual studio should detect that this is a CMake folder, and start configuring.
    1. Build as usual (press F5).

### Creating/debugging the installer

If you want to debug the installer:
1. Execute once, the `build.bat` once. 
    1. If it fails to run:
    1. From Windows terminal, `cd c:\user\epiccoder\Documents\QtEdit4`
    1. Run nanually `call build.bat` . Try to understand what went wrong,
       usually, this means some path inside the batch file is wrong. 
2. From the Windows Start menu find *Inno Setup Compiler* and urn it. When it
   starts, point it to `setup_script.iss` inside the qtedit4 clone.
3. Press F9, to compile the installer and execute it.


## Building on GNU/Linux

Follow the instructions for your distro, and then the "Shared" at 
the end. Once the code compiles, you can use QtCreator to do regular builds. I 
also recommend using a "Kit" that uses "ccache" to speed up compilations. That 
is not covered by this document.

### Debian/Ubuntu

Install the following packages:

```
sudo apt install python3-pip python3-pipx build-essential libcups2-dev clang clang-tools ninja-build cmake ccache mold
```

### Fedora

Install the following packages:

```
sudo dnf install python3-devel python3-pip \
    wayland-devel mesa-libGL-devel cups-devel \
    clang clang-tools-extra ninja cmake ccache mold
```

### Arch

Install the following packages:

```
sudo pacman -S python-pip python-pipx base-devel libcups clang clang-tools-extra ninja cmake ccache mold
```

### Shared install 

> **Note**: in this setup, we install Qt using [aqt](https://github.com/miurahr/aqtinstall)
for installing Qt. You can use the official online installer (which I find slow)
or using your distro packages.
>
> This setup seems to work for me under Fedora (gnome default
> installation) and Debian (which I use Plasma). I tested this under Manjaro
> Linux (so, basically arch...) with greate success.
>
> You may ignore this step, and install Qt6 from your repos, and you can also
> use VSCode to compile. 
>
> You are not forced to build on the command line, you can use your IDE, but it
> will require some setup. Documentation is always welcomed.
>
> While this document mentions Qt 6.9.1, it is recommended to download the latest
> version avialable. See https://doc.qt.io/qt-6/qt-releases.html for more details.
>
> Under modern ditros (Debian, and Arch) `pip` will faill. Use instead `pipx`.

From the command line (when installing Qt, `-m all` means install all modules
this might not be needed for this process - but this is a good thing to have,
execute the following code:


``` bash
pip install aqtinstall
aqt install-qt linux desktop 6.9.1 -m all -O ~/qt
aqt install-tool linux desktop tools_qtcreator_gui -O ~/qt
```
Alternativelly - you can use the Qt Online installer (its easier on Windows).

Download the source code, and start building (first exports can be added into
your `~/.bashrc` 
```
export PATH=~/qt/6.8.1/gcc_64/bin/:$PATH
export NINJA_STATUS="[%f/%t %p %P][%w + %W] "
export CC=/usr/lib64/ccache/gcc
export CXX=/usr/lib64/ccache/g++

mkdir -p src
cd src
git clone https://github.com/diegoiast/qtedit4.git 
cd qtedit4
cmake -S . -B build -G Ninja
cmake --build build
```

Notes:
1. See how we chane the default ouput from GnuMakefiles to Ninja, 
   its faster.
1. *Optional:* We installed `mold` which is a faster linke, which is auto detected.
1. *Optional:* We use `ccache` which means sequential rebuilds will be done
   in "0" time. 
1. *Optional:* We modify the output of `ninja` to display ETA
1. The `exports` you see here, can be added to your `~/.bashrc`.
1. `ccache` uses the full paths for compilation units. If you rename
   the built/output dir - the next rebuild will be a full rebuild,
   and `ccache` will not see hits. Try to avoid it.

If you want to create the AppImage (note that it does a full rebuild):
```
time ./build.sh
```

On my T14s, with i5-10210U, the whole build taks takes about 3 minutes, without
ccache (which is recommended, and will speed up the second build):

```
./build.sh  1023.52s user 39.31s system 556% cpu 3:10.93 total
```

You might need to modify it to work on your system (on Fedora, the path for
ccache compilers is `CC=/usr/lib64/ccache/gcc` and in Debian it's
`CC=/usr/lib/ccache/clang-19` for example). Do not make PRs on this
subject, as this is script is really very intimate with your system
(unless you can genenalize it to work every where). Output will be called
`dist/qtedit4-qt680-v0.0.3-x86_64.AppImage` (depending on various definitions
in the script, the output may slightly vary). Just `chmod` that file and 
you can execute it.

I found that under some systems, the Qt plugin of AppImage, will fail with erros
similar to this:

```
ERROR: Strip call failed: /tmp/.mount_linuxdAMaGia/usr/bin/strip: dist/ubuntu-gcc/usr/lib/libXau.so.6: unknown type [0x13] section `.relr.dyn'
```

The solution is to run:
```
NO_STRIP=true ./build.sh
```

## Building on macOS

**TODO**. Get Qt, and build. macOS is not supported yet, due to missing hardware.
It is compiled as part of the CICD, but no packages are created.

In theory, you should follow the same path as on Linux. However, as I don't own
an Mac, I cannot fully test this. CI/CD is testing builds, but as I don't own
a suitable machine, I cannot test how the application looks, feels nor runs.

## Modifying 3rd parties

If you find a bug in one of the 3rd parties (one of th projects added via CPM),
you have access to the code, and you can fix locally. However, not that you
just made a local fork. You shold make that fork available on the net, and 
ideally make a PR to the original project.

1. Right click on file name in QtCreator. You will see a menu, select "Copy
   full path". 
1. On your terminal, `cd` to that path, I usually paste the full file path on 
   my shell, and remove the filename. I am now inside the git clone made for 
   this build of this library.
1. `git remote add github git@github.com:epicdud/qmdilib.git` for example. 
1. Now you can modify the local copy, and push to your own repo. You can now 
   `git push github` to publish your modifications.
1. You shuld also modify the main `CMakeLists.txt` for the app, to use your repo
   until upstream takes your patch.

Alternativelly, there is a script called `get-code.sh` which will pull all cmake
dependencies into `lib`. You then need to run cmake with QTEDIT4_WORK_OFFLINE
defined. Now, you will see the 3rd party sources in lib, as normal git repos.

## Coding standards

1. Indentation is defined in a `.clangformat` file. There is a target that
   will reformat the project. Run it before any PR: `ninja -C build clangformat`
1. Format is checked on the CI/CD. 
1. Comments/text english grammar is also checked on the CI/CD.
1. When dealing with Qt code - name convention is: `PascalNotation` for classes
   and `camelCase` for methods/variables.
1. When dealing with "pure" c++ code, dealing with STL, we use `under_score` and
   always lower case for everything. 
1. `#define CONST` are upper case, like in good old C.
1. If you are masking a stand-alone funtion, use the "new" C++ syntax:
   `auto static foo(int bar, std::hash_map zoop) -> bool`. Yes, this is 
   a valid C++ syntax. 
1. If you need helper functions in your translation unit - make those funcitons
   as `static`. If you need/want to share them - add them to the header, if
   possible under a namespace.
1. Try to use `enum class`. In rare cases, use traditional `enum`. If you
   do need to use traditional `enum` when dealing with it, use it as it was
   a class enum (meaning `Colors::Red` instead of just `Red`).
1. I have a tendency to remove Qt code as much as possible. If you are creating
   new functionality - try to do this in STL and not Qt. I see Qt as an 
   implementation detail that migt change in the future.
1. NERVER create static fork projects. Meaning, do not just copy the source of a 
   project into the source tree of this project. Instead - use upstream (to 
   consume it we use CPM at the moment). This means that sometimes, We need to
   fork upstream, add to the code a CMakeLists.txt at the top - and cosume this
   fork. The idea is that we can them make a PR to that project and remove that
   temporary. CPM has also tricks to deal with projects that have no CMake 
   support. ChatGPT can help with this as well.

## Internals

### Core - qmdilib 

This library handles the modification of toolbars and menus. Each widget you 
want to put into the main tab, it should derive `qmdiWidget`. You should assign
`this->menus` and `this->toolbars['main']` with the actions you want to be
displayed when the widget is on focus.

This library also handles the configuration (how entries are saved in the ini 
files) and how the GUI is displayed, and also plugin development.

To understand how configuration works, see this demo: https://github.com/diegoiast/qmdilib/blob/main/demos/demo3/main3.cpp

#### New widgets
To understand how to create a new widget, look into  `src/plugins/imageviewer/imageviewer_plg.cpp`.
The core of the idea is to derive your new widget from the widget you like to
integrate and `qmdiWidget`. Then set `mdiClientName` to the filename you loaded,
and add actions to the toolbars/menus as needed.

#### New plugins
Plugins are the thing to add new menus, toolbars and side panels to the main UI.
When a file is loaded, the system asks each plugin "can you load this file?".
Each plugin then gives a score, and the plugin with the highest score will be
given the opportunity to create a `qmdiWidget` to "edit" the file.

Examples:
1. If you try to open a file that ends with `*.txt`, all plugins will get
   queried by the method `int IPlugin::canOpenFile(const QString)`.
    1. The text editor plugin will return score of 5
    1. The hex viewer plugin will return score of 2
    1. The text editor plugin wins, and `bool openFile(const QString fileName, int x, int y, int z)`
       will be called.
    1. The text editor plugin will create a QutePart editor, and will open the 
       file. The arguments `x` and `y` note the cursor position. You can ignore 
       the `z` argument for now.
1. If you open a `*.img` file - the hex viewer plugin will win with score of 5. 
   it will open `QHexView` (see the 3rd parties in the CMakeLists.txt).
1. If you open a `*.jpg` file
    1. Hex plugin will return 2
    1. Text plugin will return 1
    1. Image viewer will return 5.

To understand how to open a new file from the plugins - the image viewer or 
hex-viewewr plugins are the best to learn.

If you want to gobally configure your widgets from the plugin, each plugin
as a `this->config`. See for example how the `HelpPlugin` does this. Its simple
enough to understand.

#### Changing how plugins/widget work

This should not be done  inside QtEdit4. If you want to modify how this works
all changes must be done inside qmdilib, look for `PluginDemo`. That is a small
scale application that behaves like qtedit4. Once you understand that demo,
you will see that QtEdit4 is just a different set of plugins loaded.

### Example plugin - search

To understand how to make a simple plugin, look at the search. It creates a 
docking area, and adds a widget there. Then all actions are done inside the
created widget.

The to communicate to the "system", that a file should be open it does:
```
auto host = dynamic_cast<PluginManager *>(parent);

// assume fileName="/etc/hosts" and line=12
host->openFile(fileName, line);
host->focusCenter();
```

### Text editor - qutepart

The current plugin that handles text files does this by creating a 
`QutePart` editor. You can create a new plugin, that opens a different text
editor component, returning a higher score for text files.

The text editing component is made of 2 parts: the editor itself, and the 
highlighting engine. The reason is just the wat that Qt works. Most of the work
should be done in `QutePart` library. 

The widget created by this plugin, is a compound widget that contains the editor
and a toolbar to display the cursor position, and choosing the indentation/theme
used. 

### Project management

UI which shows the root of the project. 
Building is done by a plugin. That plugin just opens a side panel, and it has a
way to decide which commands to run to build.

Each projet has 2 different axis:
1. The tasks available: which commands should be run, for example configure,
   build or deploy. There is also a task that runs the executable.
1. Kits: The environment to be used for running those tasks. Environment can be 
   different rust versions, or using clang/MSVC. Kits are scripts (`.sh` on 
   Unix, `.bat` on Windows) that usually set environment variables - and then 
   call the needed task.
   
Kits are all scripts files in the file system (can be auto generated).
Tasks is a JSON file at the root of the project. The IDE can auto detect tasks
for some projects (rust projects are simple example, `cargo build` to compile 
the project, and `cargo run` to execute it).

Look for the files `kitdetector.h`, `kitdefinitions.h` `ProjectBuildConfig.h`
for understand that subsystem.
