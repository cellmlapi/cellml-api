all: DOMImplementation.o DOMWriter.o DOMBootstrap.o

DOMImplementation.o: DOMImplementation.cpp
	g++ -Wall -ggdb -I../../interfaces -I.. `pkg-config --cflags glib-2.0` `pkg-config --cflags gdome2` -c DOMImplementation.cpp

DOMBootstrap.o: DOMBootstrap.cpp
	g++ -Wall -ggdb -I../../interfaces -I.. `pkg-config --cflags glib-2.0` `pkg-config --cflags gdome2` -c DOMBootstrap.cpp


DOMWriter.o: DOMWriter.cpp
	g++ -Wall -ggdb -I../../interfaces -I.. `pkg-config --cflags glib-2.0` `pkg-config --cflags gdome2` -c DOMWriter.cpp