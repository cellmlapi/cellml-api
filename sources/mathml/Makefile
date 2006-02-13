all: MathMLImplementation.o

MathMLImplementation.o: MathMLImplementation.cpp
	g++ -Wall -ggdb -I../../interfaces -I../dom -I.. `pkg-config --cflags glib-2.0` `pkg-config --cflags gdome2` -c MathMLImplementation.cpp