#! /bin/bash 

if [ ! -d boost_1_73_0 ]; then
    wget https://dl.bintray.com/boostorg/release/1.73.0/source/boost_1_73_0.zip
    unzip boost_1_73_0.zip
    cd boost_1_73_0
    ./bootstrap.sh
    ./b2 install --prefix=../..
fi
