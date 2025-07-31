# July 2025 - release - alpha1

This month continues the work of making the IDE better for editing. Split tabs
are working better, can be considered stable. Added meson support. Where supported
by the build system, we have real target support (in cmake, we can detect the
exes found, instead of guessing it). The editor got multiple cursor support (my 
favorite feature so far!).

* Added meson support: https://github.com/diegoiast/qtedit4/issues/96 https://github.com/diegoiast/qtedit4/issues/94, coded live: https://youtu.be/HQQUMf89cVg
* Project search not clickable: https://github.com/diegoiast/qtedit4/issues/93
* Added support for multiple cursors: https://github.com/diegoiast/qutepart-cpp/issues/18
* Search path is not saved: https://github.com/diegoiast/qtedit4/issues/85
* Split tabs - move beween tabs, click on empty place for new file: https://github.com/diegoiast/qtedit4/issues/62 https://github.com/diegoiast/qtedit4/issues/91
* Settings- reset to defaults: https://github.com/diegoiast/qmdilib/issues/20, see also
* Curstom project configuration updates : https://github.com/diegoiast/qtedit4/commit/97b7ab49d63939ae6fe1802c5e6f397297622272, https://github.com/diegoiast/qtedit4/commit/f54117983a30a28716aacc5782e3477e202aa523