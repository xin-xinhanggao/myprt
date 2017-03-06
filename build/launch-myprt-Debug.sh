#!/bin/sh
bindir=$(pwd)
cd /Users/apple/Desktop/myprt/myprt/
export 

if test "x$1" = "x--debugger"; then
	shift
	if test "xYES" = "xYES"; then
		echo "r  " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		/usr/local/bin/gdb -batch -command=$bindir/gdbscript --return-child-result /Users/apple/Desktop/myprt/build/Debug/myprt 
	else
		"/Users/apple/Desktop/myprt/build/Debug/myprt"  
	fi
else
	"/Users/apple/Desktop/myprt/build/Debug/myprt"  
fi
