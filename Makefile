CPP = g++
LD = g++
CPPFLAGS = -O3 -std=c++11 -g
LDFLAGS =
TARGET = pyramid
OBJS = objs/main.o objs/setup.o objs/controls.o objs/mesh.o objs/model.o objs/scene.o objs/lightstate.o objs/scattering.o
INCLUDE = -I/usr/local/include
OS = $(shell uname)

ifeq ($(OS),Darwin)
	LIB =  -framework OpenGL -framework Cocoa -lGLEW -lglfw3 
else
	LIB =  /usr/local/lib/libSOIL.a -lglfw -lGL -lGLEW  -lassimp 
endif

default: $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) $(LIB) -o $(TARGET) 

objs/main.o: main.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) main.cpp -o objs/main.o

objs/setup.o: common/setup.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) common/setup.cpp -o objs/setup.o

objs/controls.o: common/controls.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) common/controls.cpp -o objs/controls.o

objs/mesh.o: common/mesh.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) common/mesh.cpp -o objs/mesh.o

objs/model.o: common/model.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) common/model.cpp -o objs/model.o

objs/scene.o: scene.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) scene.cpp -o objs/scene.o

objs/lightstate.o: lightstate.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) lightstate.cpp -o objs/lightstate.o

objs/scattering.o: scattering/scattering.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDE) scattering/scattering.cpp -o objs/scattering.o

clean:
	rm -f objs/*.o $(TARGET)
