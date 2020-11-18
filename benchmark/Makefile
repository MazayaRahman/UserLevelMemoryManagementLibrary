all: test

test:
	gcc test.c -L../ -lmy_vm -lm -m32 -o test

testThread: 
	gcc testThread.c -L../ -lmy_vm -lm -lpthread -m32 -o testThread

clean:
	rm -rf test testThread
