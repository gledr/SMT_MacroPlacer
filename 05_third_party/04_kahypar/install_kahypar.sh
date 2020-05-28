#! /bin/bash 

if [ ! -d kahypar ]; then
    git clone --depth=1 --recursive https://github.com/kahypar/kahypar
    cd kahypar
    git checkout 1.9.2
    mkdir build
    cd build
  
    cmake -DCMAKE_INSTALL_PREFIX=../../../ ..

    make -j 2
    make install
fi
