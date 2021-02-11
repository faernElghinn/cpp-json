#!/bin/bash

set -e

j=8;
if [[ "$1" == *1* ]] ; then
    j=1;
fi

mkdir -p Build > /dev/null
cd Build
export CXX=/usr/bin/g++-9
export CC=/usr/bin/gcc-9
cmake .. -DCMAKE_BUILD_TYPE=DEBUG &&
make -j "$j" && 
ctest --output-on-failure && 
sudo make install &&
sudo ldconfig
cd ..
