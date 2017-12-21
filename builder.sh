#!/bin/sh
export PLATFORM_NAME=$1
if [ ! -e BuildFor$1.cmake ] ;
then
    echo -e "Invalid platform [$1]!"
    echo -e "Valid platforms include:\n"
    BUILDERS=`ls ../BuildFor*`
    for i in $BUILDERS
    do
        echo $i | sed -e 's/\.\.\/BuildFor\(.*\).cmake/\1/'
    done
else
BUILD_DIR=build.$PLATFORM_NAME

if [ -d $BUILD_DIR ]; then
    echo "Build dir exists, erase [y/N]"
    read res
    if [ "$res" = "y" ]; then
        echo "erasing build dir"
        rm -rf $BUILD_DIR
    else
        echo "aborting!"
        exit 1
    fi
fi

mkdir $BUILD_DIR
cd $BUILD_DIR
cmake  -DCMAKE_TOOLCHAIN_FILE=../BuildFor$PLATFORM_NAME.cmake ../
fi
