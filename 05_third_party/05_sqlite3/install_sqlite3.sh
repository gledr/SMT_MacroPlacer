#! /bin/bash 

lib_name=libsqlite3.a

if [ ! -d  $lib_name ]; then
    home_dir=$(pwd)
    
  
    gcc -c sqlite3.c
    ar -crs $lib_name sqlite3.o
    
   
    cp $lib_name ../lib
    cp sqlite3.h ../include
   
    cd $home_dir
fi
