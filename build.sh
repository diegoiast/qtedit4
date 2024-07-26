#! /bin/sh

set -x
set -e

export matrix_config_build_dir=ubuntu-gcc
export PATH=$HOME/qt/6.7.1/gcc_64/bin:$PATH
export LD_LIBRARY_PATH=$HOME/qt/6.7.1/gcc_64/lib:$LD_LIBRARY_PATH

rm -fr "build/${matrix_config_build_dir}"
rm -fr "dist"

cmake -B  "build/${matrix_config_build_dir}" -DCMAKE_BUILD_TYPE=Release
cmake --build   "build/${matrix_config_build_dir}" --parallel --config Release
cmake --install "build/${matrix_config_build_dir}" --prefix dist/${matrix_config_build_dir}/usr

QTDIR="$HOME/qt/6.7.1/gcc_64/"
mkdir -p "dist/${matrix_config_build_dir}/usr/plugins/iconengines/"
mkdir -p "dist/${matrix_config_build_dir}/usr/plugins/platforms/"
cp -arv $QTDIR/plugins/iconengines/* "dist/${matrix_config_build_dir}/usr/plugins/iconengines/"
cp -arv $QTDIR/plugins/platforms/libqwayland-*.so "dist/${matrix_config_build_dir}/usr/plugins/platforms/"
# unset QTDIR

export LD_LIBRARY_PATH=dist/${matrix_config_build_dir}/usr/lib:$LD_LIBRARY_PATH
export OUTPUT=build/qtedit4-qt671-v0.0.1-x86_64.AppImage
wget -nc --no-verbose "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
wget -nc --no-verbose "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"
chmod +x linuxdeploy*.AppImage
./linuxdeploy-x86_64.AppImage --appdir "dist/${matrix_config_build_dir}" --plugin qt --output appimage
