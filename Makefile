#make file - this is a comment section
 
CC=clang  #compiler
TARGET=build-$(ARCH)/disasteroids3d #target file name

all:    #target name
	$(CC) -arch $(ARCH) 3dgame.cpp 3dglasses.cpp command.cpp commands.cpp console.cpp cvars.cpp menus.cpp renderflags.cpp timer.cpp tokenize.cpp -w -I/Users/tomkidd/Documents/GitHub/MSPStore/include/SDL2 -L/Users/tomkidd/Documents/GitHub/MSPStore/lib -lSDL2 -lSDL2_mixer -o $(TARGET) -lm -framework OpenGL -mmacosx-version-min=10.7

clean:
	rm $(TARGET)