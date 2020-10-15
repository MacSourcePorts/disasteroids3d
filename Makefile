#make file - this is a comment section
 
CC=gcc  #compiler
TARGET=disasteroids3d-Linux #target file name

all:    #target name
	$(CC) 3dgame.cpp 3dglasses.cpp command.cpp commands.cpp console.cpp cvars.cpp menus.cpp renderflags.cpp timer.cpp tokenize.cpp -w -I/usr/include/SDL2 -lSDL2 -lSDL2_mixer -o $(TARGET) -lm -lGL

clean:
	rm $(TARGET)