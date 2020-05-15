#! /bin/bash 

 dir_name=sqlite-amalgamation-3300100

if [ ! -d $dir_name ]; then
    home_dir=$(pwd)
   
    lib_name=libsqlite3.a
    wget https://sqlite.org/2019/sqlite-amalgamation-3300100.zip
    unzip sqlite-amalgamation-3300100.zip 
   
    cd $dir_name
   
    gcc -c sqlite3.c
    ar -crs $lib_name sqlite3.o
    
   
    cp $lib_name ../../lib
    cp sqlite3.h ../../include
   
    cd $home_dir
    rm *.zip
fi
