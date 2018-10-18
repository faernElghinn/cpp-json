#!/bin/bash

set -e

j=8;
if [[ "$1" == *1* ]] ; then
    j=1;
fi

mkdir -p Build > /dev/null
cd Build
export CXX=/usr/bin/g++-8
export CC=/usr/bin/gcc-8
cmake .. -DCMAKE_BUILD_TYPE=DEBUG &&
make -j "$j" && 
make test && 
sudo make install 
cd ..
