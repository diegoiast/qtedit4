#! /bin/bash

set -x
set -e

VERSION=v0.0.4
OUTPUT=qtsourceview-${VERSION}
OUTPUT_FILE=${OUTPUT}.tar.gz
TEMP=$(mktemp -d /tmp/${OUTPUT}-XXXXXXXX)
#mkdir ${TEMP}

git archive fix-documentation --format=tar.gz --prefix=${OUTPUT}/ > ${OUTPUT_FILE}
pushd .
mv ${OUTPUT_FILE} ${TEMP}
cd ${TEMP}
tar xf ${OUTPUT_FILE}
cd ${OUTPUT}
doxygen

cd data/qate
./fetch.sh
cd ../../


doxygen
cd ..
tar -cf ${OUTPUT_FILE} ${OUTPUT}
popd
mv ${TEMP}/${OUTPUT_FILE} .
rm -fr ${TEMP}