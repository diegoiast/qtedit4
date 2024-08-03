#! /bin/sh

# @@ name = Qt 6.7.1 in ~/qt
# @@ author = qtedit4

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
