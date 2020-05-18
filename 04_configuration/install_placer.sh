#==================================================================
# Author       : Pointner Sebastian
# Company      : Johannes Kepler University
# Name         : SMT Macro Placer
# Workfile     : install_placer.sh
#
# Date         : 18.May 2020
# Copyright    : Johannes Kepler University
# Description  : Script to Install the Tool on the Host System
#==================================================================
#! /bin/bash

python extract_version.py

cd ..
BASE_PATH=$(pwd)
INSTALL_PATH=~/.smt_placer


if [ -d $INSTALL_PATH ]; then
    rm -rf $INSTALL_PATH
fi

mkdir $INSTALL_PATH
cd $INSTALL_PATH
touch config.txt
echo "base_path="$BASE_PATH >> config.txt

if [ -d $BASE_PATH/01_src/version.hpp ]; then
    rm $BASE_PATH/01_src/version.hpp
fi

mv $BASE_PATH/04_configuration/version.hpp $BASE_PATH/01_src/
