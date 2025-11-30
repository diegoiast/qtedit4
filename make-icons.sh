#!/bin/bash

# Usage: ./make_icon.sh icon_name
# Requires: inkscape, imagemagick (convert)

set -e

if [ -z "$1" ]; then
    echo "Usage: $0 icon_name"
    exit 1
fi

#convert -density 300 $1.svg -define icon:auto-resize="16,32,48,64,128,256,512" $1.ico


ICON_NAME="$1"
SIZES=(16 32 48 64 128 256 512)
rm -f tmp_${ICON_NAME}_*.png

echo -n "Generating "
for SIZE in "${SIZES[@]}"; do
    echo -n "$SIZE "
    inkscape "${ICON_NAME}.svg" \
        --export-type=png \
        --export-filename="tmp_${ICON_NAME}_${SIZE}.png" \
        -w $SIZE -h $SIZE
done

echo -n ", creating ${ICON_NAME}.ico..."
convert tmp_${ICON_NAME}_*.png "${ICON_NAME}.ico"
rm -f tmp_${ICON_NAME}_*.png
echo "done."
