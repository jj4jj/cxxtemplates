
install:libxctmp.a src/xctmp.h
	mkdir -p include
	mkdir -p lib
	mv libxctmp.a lib/
	cp -f src/xctmp.h include/

test:./test/test.cpp lib/libxctmp.a
	g++ test/test.cpp -Iinclude -Llib/ -lxctmp -o test/test --std=c++11 -g3 -Wall

libxctmp.a:src/xctmp.cpp
	g++ -c src/xctmp.cpp --std=c++11 -I. -g -O2 -Wall
	ar -rcs libxctmp.a xctmp.o
	rm -f xctmp.o

clean:
	rm -f *.o *.a test
