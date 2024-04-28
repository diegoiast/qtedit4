#! /bin/bash

set -x
set -e

VERSION=$(git describe --tags)
OUTPUT=qmdilib-${VERSION}
OUTPUT_FILE=${OUTPUT}.tar.gz
TEMP=$(mktemp -d /tmp/${OUTPUT}-XXXXXXXX)
#mkdir ${TEMP}

git archive master --format=tar.gz --prefix=${OUTPUT}/ > ${OUTPUT_FILE}
pushd .
mv ${OUTPUT_FILE} ${TEMP}
cd ${TEMP}
tar xf ${OUTPUT_FILE}
cd ${OUTPUT}
doxygen
cd ..
tar -cf ${OUTPUT_FILE} ${OUTPUT}
popd
mv ${TEMP}/${OUTPUT_FILE} .
rm -fr ${TEMP}
