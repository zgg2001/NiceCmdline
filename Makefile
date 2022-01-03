.PHONY: all install

all: libnice.so
libnice.so: 
	mkdir -p build
	gcc -fPIC -shared -o build/libnice.so nice_cmd/*.c -I ./

install:
	mkdir -p /usr/local/include/nice_cmd
	cp ./nice_cmd/*.h /usr/local/include/nice_cmd
	cp ./build/libnice.so /usr/local/lib/
