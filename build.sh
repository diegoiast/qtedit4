#! /bin/sh

set -x
set -e

QTDIR="$HOME/qt/6.7.1/gcc_64/"
export matrix_config_build_dir=ubuntu-gcc
export PATH=$QTDIR/bin:$PATH
export LD_LIBRARY_PATH=$QTDIR/lib:$LD_LIBRARY_PATH
export OUTPUT=dist/qtedit4-qt671-v0.0.2-x86_64.AppImage

rm -fr "build/${matrix_config_build_dir}"
rm -fr "dist"

cmake -B "build/${matrix_config_build_dir}" -DCMAKE_BUILD_TYPE=Release
cmake --build   "build/${matrix_config_build_dir}" --parallel --config Release
cmake --install "build/${matrix_config_build_dir}" --prefix dist/${matrix_config_build_dir}/usr

# Qt plugin does not bundle the needed plugins, do this manually
mkdir -p "dist/${matrix_config_build_dir}/usr/plugins/iconengines/"
mkdir -p "dist/${matrix_config_build_dir}/usr/plugins/platforms/"
cp -arv $QTDIR/plugins/iconengines/* "dist/${matrix_config_build_dir}/usr/plugins/iconengines/"
cp -arv $QTDIR/plugins/platforms/libqwayland-*.so "dist/${matrix_config_build_dir}/usr/plugins/platforms/"

wget -nc --no-verbose "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
wget -nc --no-verbose "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"
chmod +x linuxdeploy*.AppImage
./linuxdeploy-x86_64.AppImage --appdir "dist/${matrix_config_build_dir}" --plugin qt --output appimage
