#==================================================================
# Author       : Pointner Sebastian
# Company      : Johannes Kepler University
# Name         : SMT Macro Placer
# Workfile     : extract_version.py
# Interpreter  : Python 3.8.2  
#
# Date         : 18.May 2020
# Copyright    : Johannes Kepler University
# Description  : Script to extract current Git Information
#==================================================================

#! /bin/env/python

import os

DATE='git log -1 --date=short --pretty=format:%cd'
HASH='git rev-parse HEAD'
NAME='git log -1 --pretty=format:\'%an\''

class CodeGenerator:
    def __init__(self, filename):
        self.__filename = filename

    def set_date(self, date):
        self.__date = date
        
    def set_name(self, name):
        self.__name = name
    
    def set_hash(self, hashval):
        self.__hash = hashval
        
    def generate(self):
        outfile = open(self.__filename, "w+")
        outfile.write("#ifndef PLACER_VERSION_HPP\n")
        outfile.write("#define PLACER_VERSION_HPP\n")
        outfile.write("/*\n")
        outfile.write(" * Generated File! Do not modify!\n")
        outfile.write("*/\n")
        outfile.write("\n")
        outfile.write("#include <string>\n")
        outfile.write("\n")
        outfile.write("namespace Placer {\n")
        outfile.write("\n")
        outfile.write("std::string const GIT_HASH=\""  + str(self.__hash) + "\";\n")
        outfile.write("std::string const GIT_DATE=\""  + str(self.__date) + "\";\n")
        outfile.write("std::string const GIT_NAME=\""  + str(self.__name) + "\";\n")
        outfile.write("\n")
        outfile.write("}\n")
        outfile.write("\n")
        outfile.write("#endif /*PLACER_VERSION_HPP*/\n")
        outfile.write("\n")
        
        outfile.close()
        

if __name__ == "__main__":
    code_gen = CodeGenerator("version.hpp")
    __date = os.popen(DATE).read()
    __hash = os.popen(HASH).read()
    __name = os.popen(NAME).read()
    
    code_gen.set_date(__date)
    code_gen.set_name(__name)
    code_gen.set_hash(__hash[:-1])
    code_gen.generate()
