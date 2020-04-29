SDLFLAGS = $(shell sdl2-config --libs --cflags)

game:
	mkdir -p build
	rm -f build/main
	g++ src/*.cpp -o build/main $(SDLFLAGS) -lSDL2_mixer -lSDL2_image -lSDL2_ttf -framework OpenGL -std=c++17 -g
