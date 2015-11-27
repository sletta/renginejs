OSNAME          := $(shell uname -s)
DUKTAPE_DIR     := ../duktape-1.3.0
RENGINE_DIR     := ../rengine

INCLUDES        := -I$(RENGINE_DIR)/include -I$(RENGINE_DIR)/3rdparty -I/usr/local/include/SDL2 -I$(DUKTAPE_DIR)/src
DEFINES         := -DRENGINE_OPENGL_DESKTOP -DRENGINE_BACKEND_SDL
CFLAGS          := -g
CXXFLAGS        := -std=c++11

ifeq ($(OSNAME),Darwin)
    OS               := =osx
    CXX              := clang++
    CC               := clang
    LFLAGS           := -framework OpenGL -L/usr/local/lib -lSDL2
else
    OS               := linux
    CXX              := g++
    CC               := gcc
    LFLAGS           :=
endif

all: jsrengine

jsrengine: duktape.o main.o
	$(CXX) duktape.o main.o -o jsrengine $(LFLAGS)

duktape.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) $(DEFINES) $(DUKTAPE_DIR)/src/duktape.c -o duktape.o

main.o: main.cpp
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) $(INCLUDES) $(DEFINES)  main.cpp -o main.o

clean:
	rm -f *.o
	rm -f jsrengine
