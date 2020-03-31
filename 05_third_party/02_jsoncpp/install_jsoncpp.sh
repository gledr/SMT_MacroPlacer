#! /bin/bash 

if [ ! -d jsoncpp ]; then
    git clone https://github.com/open-source-parsers/jsoncpp 
    cd jsoncpp
    git checkout 1.9.2
    mkdir build
    cd build
  
    cmake -DCMAKE_INSTALL_PREFIX=../../../ ..


    make -j 2
    make install
fi
