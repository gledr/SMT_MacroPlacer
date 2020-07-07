#! /bin/bash 

if [ ! -d z3 ]; then
    git clone https://github.com/Z3Prover/z3
    cd z3
    git checkout z3-4.8.7
    python scripts/mk_make.py --prefix=../../../
    cd build
    make -j 2
    make install
fi
