P3: mem.so main.c
	cc -lmem -L. -o P3 main.c libmem.so -Wall -Werror
	echo "Attention: use \"export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.\" before execution!"

mem.so: mem.c mem.h
	cc -shared -fPIC mem.c -o libmem.so -Wall -Werror
