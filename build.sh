#!/bin/bash

(
  	cd "$(dirname "$0")"
  	echo "Currently in: $(pwd)"
	
	echo "
	Helios Builder, Please Make sure you already have uWebsockets (and uSockets), zlib, pthread.
	./build.sh [args]
parse arg: 
	0 -> to build from internal libs(if you have installed)
	1 -> to build from conan packages"

	echo "
		-----------------------------------------------------
		starting Build
		-------------------------------------------------------------
"

	if [ "$1" -eq 0 ]; then
		cmake --build build
		mv -f build/helios examples/helios/
	elif [ "$1" -eq 1 ]; then
		cmake --build build --preset conan-release
		mv -f build/build/Release/helios examples/helios/
	else
    		echo "Invalid Build Arg"
	fi

echo "done Building Boss."
)
# When the ) is reached, you are automatically "back" where you started

