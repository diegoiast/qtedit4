**Full Changelog**: https://github.com/diegoiast/qtedit4/compare/v0.0.11...v0.0.12

# August 2025 - release 

While building on Windows on qtedit4 I found some problems:

1. In the cmake stage, there are lots of warnings about symlinks not supported
   this spammed the build log with several (about 800,000 bytes!) of HTML. This 
   was fixed by using the rich text API. In this video (https://youtu.be/TQyRpQ4oc3E)
   I show this is working now. 
2. While loading the project, the UI was effectivly frozen. Again, under Windows
   the UI would be locked for 30 seconds on some setups (mine...). I fixed it by
   fixing the widget that loads the project into truely working on a background
   thread.
3. If you had several projects loaded, ctags loading was done in the main thread
   which in my setup, would account for 1.5 seconds. Fixed live in 
   https://youtu.be/TQyRpQ4oc3E

All of those issues are fixed now.


Other interesting issues fixed:

1. Editor behaviour (moving lines, copy/cut) is closer to VSCode.
2. When running a compiled executable, you can run it on different build 
  .environments/kits. See https://github.com/diegoiast/qtedit4/issues/47, this 
   was visible on this video: https://youtu.be/T7Ao5JWkLPc?si=D5WLba7UZGExyP5S&t=215
3. The json configuration file syntax has been updated, now you can have 
   platfrom specific commands. 
4. Editor perforceman gains, by porting code to newer C++ syntax.

## Changelog

 * CTags loading slows down main gui -  https://github.com/diegoiast/qtedit4/issues/109
 * ProjectManager: log output is slow - https://github.com/diegoiast/qtedit4/issues/102
 * Project manager: configure fails on second time - https://github.com/diegoiast/qtedit4/issues/108
 * Completer::updateWordSet is called too much - https://github.com/diegoiast/qutepart-cpp/issues/35
 * Qutepart: control+x - cut current line - https://github.com/diegoiast/qutepart-cpp/issues/40
 * Qutepart: move selected lines - https://github.com/diegoiast/qutepart-cpp/issues/39
 * Project manager: add support for different environment on build/run - https://github.com/diegoiast/qtedit4/issues/47
 * ProjectManager: files loading is slow - https://github.com/diegoiast/qtedit4/issues/103
 * editor: trimming breaks the document - https://github.com/diegoiast/qtedit4/issues/104
 * ProjectManager: white background for log output - https://github.com/diegoiast/qtedit4/issues/110
 * TextOperationWidget: input font should folow text editor - https://github.com/diegoiast/qtedit4/issues/112
 * Qutepart/meta: remove usage of foreach - https://github.com/diegoiast/qutepart-cpp/issues/43
