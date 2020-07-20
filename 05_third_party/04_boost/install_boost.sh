#! /bin/bash 

if [ ! -d boost_1_73_0 ]; then
    wget https://sourceforge.net/projects/boost/files/boost/1.73.0/boost_1_73_0.tar.gz
    tar xf boost_1_73_0.zip
    cd boost_1_73_0
    ./bootstrap.sh
    ./b2 install --prefix=../..
fi
