#! /bin/bash 

if [ ! -d  optimathsat-1.6.3-linux-64-bit ]; then
    wget http://optimathsat.disi.unitn.it/releases/optimathsat-1.6.3/optimathsat-1.6.3-linux-64-bit.tar.gz
    tar xf optimathsat-1.6.3-linux-64-bit.tar.gz
  
	if [ ! -d ../bin ]; then
		mkdir ../bin
	fi

	ln -sf $(pwd)/optimathsat-1.6.3-linux-64-bit/bin/optimathsat ../bin/
fi
