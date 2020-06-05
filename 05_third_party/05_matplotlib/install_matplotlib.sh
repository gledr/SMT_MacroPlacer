#! /bin/bash 

if [ ! -d matplotlib ]; then
    git clone https://github.com/lava/matplotlib-cpp
    cd matplotlib-cpp
   
    cp matplotlibcpp.h ../../include/
fi
