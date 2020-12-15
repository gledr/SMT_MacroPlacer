#! /bin/bash 

lib_name=libsqlite3.a

if [ ! -e  ../lib/${lib_name} ]; then
	echo "Compiling SQlite3..."
    home_dir=$(pwd)
  
    gcc -c sqlite3.c
    ar -crs $lib_name sqlite3.o
    
   
    cp $lib_name ../lib
    cp sqlite3.h ../include
	
    cd $home_dir
	rm *.o
	rm *.a

else
	echo "SQlite3 already installed and ready to go :)"
fi
