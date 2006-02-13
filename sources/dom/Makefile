all: DOMImplementation.o

DOMImplementation.o: DOMImplementation.cpp
	g++ -Wall -ggdb -I../../interfaces -I.. `pkg-config --cflags glib-2.0` `pkg-config --cflags gdome2` -c DOMImplementation.cpp