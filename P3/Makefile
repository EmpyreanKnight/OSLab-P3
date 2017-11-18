make: mem.so 

mymain:
	cc -lmem -L. -o myprogram mymain.c libmem.so -Wall -Werror
	echo "Attention: please type \"export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.\" before execution!"

mem.so: mem.c mem.h
	cc -shared -fPIC mem.c -o libmem.so -Wall -Werror
