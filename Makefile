install:libxctmp.a xctmp.h
	mkdir -p xctmp/include
	mkdir -p xctmp/lib
	cp -f libxctmp.a xctmp/lib/
	cp -f xctmp.h xctmp/include/

test: libxctmp.a test.cpp install
	g++ test.cpp -Lxctmp/lib/ -lxctmp -o test --std=c++11 -g -Wall

libxctmp.a: xctmp.cpp
	g++ -c xctmp.cpp --std=c++11 -g3 -Wall
	ar r libxctmp.a xctmp.o
	rm -f xctmp.o

clean:
	rm -f *.o *.a test
