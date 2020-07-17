#! /bin/bash 

root=$(pwd)

if [ ! -d MiniZincIDE-2.4.3-bundle-linux-x86_64 ]; then
    wget https://github.com/MiniZinc/MiniZincIDE/releases/download/2.4.3/MiniZincIDE-2.4.3-bundle-linux-x86_64.tgz
    tar xf MiniZincIDE-2.4.3-bundle-linux-x86_64.tgz
    
    if [ ! -d ../bin ]; then
        mkdir ../bin
    fi
    
    cd ../bin
    ln -sf ../06_minizinc/MiniZincIDE-2.4.3-bundle-linux-x86_64/bin/minizinc .
fi

cd $root

if [ ! -d or-tools_flatzinc_Ubuntu-20.04-64bit_v7.7.7810 ]; then
    wget https://github.com/google/or-tools/releases/download/v7.7/or-tools_flatzinc_ubuntu-20.04_v7.7.7810.tar.gz
    tar xf or-tools_flatzinc_ubuntu-20.04_v7.7.7810.tar.gz
    
    if [ ! -d ../bin ]; then
        mkdir ../bin
    fi
    
    cd ../bin
    ln -sf ../06_minizinc/or-tools_flatzinc_Ubuntu-20.04-64bit_v7.7.7810/bin/fzn-or-tools .
fi
