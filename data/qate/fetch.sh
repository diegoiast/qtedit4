#! /bin/sh

#set -x
set -e

# download syntax files
ROOT=https://kate-editor.org/syntax/data/
files=`wget $ROOT -q -O - | grep xml | awk '{print $5'} | cut -d\" -f 2`

ROOT=https://kate-editor.org/syntax/update-5.60.xml
files=`wget $ROOT -q -O - | grep url | awk '{print $3}' | grep url | cut -f2 -d=`
for f in $files; do
    printf  "Downloading $f"
    ff=`echo "$f" | sed -e 's/\"//g'`
    wget $ff --no-clobber -q
    printf ".\n"
done

# download themes
ROOT=https://kate-editor.org/syntax/data/themes/
files=`wget $ROOT -q -O - | grep '.theme' | awk '{print $6'} | cut -d\" -f 2`
files=`wget $ROOT  -q -O - | egrep '[.]+theme' | awk '{print $6'} | cut -d\" -f 2`
for f in $files; do
    printf  "Downloading $f"
    wget $ROOT/$f -q --no-clobber
    printf ".\n"
done
mv *.theme ../colors/
