all: CellMLImplementation.o CellMLBootstrap.o

CellMLImplementation.o: CellMLImplementation.cpp
	g++ -Wall -ggdb -I../../interfaces -I../dom -I.. `pkg-config --cflags glib-2.0` `pkg-config --cflags gdome2` -c CellMLImplementation.cpp

CellMLBootstrap.o: CellMLBootstrap.cpp
	g++ -Wall -ggdb -I../../interfaces -I../dom -I.. `pkg-config --cflags glib-2.0` `pkg-config --cflags gdome2` -c CellMLBootstrap.cpp
