[![Build Status](https://travis-ci.com/gledr/SMT_MacroPlacer.svg?branch=master)](https://travis-ci.com/gledr/SMT_MacroPlacer)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
# SMT Macro Placer
VLSI MacroPlacer for System on Chip Design

Macro Placement for System on Chip design based on solving engines for the Satisfiability Modulo Theories  like z3. 

![](./06_doc/placement_inverter_chain_0_shrink.png)

SMT_MacroPlacer heavily depends on external modules and libraries. External modules used in this build are getting resolved automatically. External libraries have to provided by the user.



The tool is developed using Arch Linux and is tested using CI for Ubuntu 20.04 LTS.

In oder to resolve the packages needed for Ubuntu 20.04 LTS, please make sure the following packages have been installed:

- git
- libboost-program-options-dev
- libboost-filesystem-dev
- g++
- doxygen
- graphviz
- bison
- zlib1g-dev
- python-numpy (Python2)
- python2-dev

If the package manager can not resolve the Python2 packages, please add the **universe** repository to your system via "apt-add-repository universe".



SMT_MacroPlacer is developed at the Institute for Integrated Circuits at the Johannes Kepler University Linz, Austria in cooperation with Infineon Technologies Duisburg, Germany.

