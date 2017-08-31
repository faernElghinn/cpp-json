#!/bin/bash

set -e

j=8;
if [[ "$1" == *1* ]] ; then
    j=1;
fi

mkdir -p Build > /dev/null
cd Build
cmake .. -DCMAKE_BUILD_TYPE=DEBUG &&
make -j "$j" && 
sudo make install 
cd ..
