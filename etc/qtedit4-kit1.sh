#! /bin/sh

# This is a kit definition for qtedit4. All your tasks
# will run trought this file.

# available enritonment variables:
# %source_directory% - where the source being executed is saved
# %build_directory%  - where code should be compile into
# %run_directory%    - where this taks should be run, as defined in
#                      the task's definition in the JSON file
# %task%             - the actual code to be run

# The following meta variables are for qtedit4. Note the prefix:
# @@ name = Qt 6.7.1 in ~/qt
# @@ author = auto generated - by qtedit4

# from this point on - echo is on. Every command will be displayed
# in the build output.
set -x
set -e

cd ${run_dir}
echo "Running from kit ${0}"
echo "Source is in ${source_dir}"
echo "Binaries will be in ${build_dir}"

# qt6 found
export QT6DIR=/home/diego/qt/6.7.1/gcc_64
export PATH=$QT6DIR/bin:$PATH

${task}
