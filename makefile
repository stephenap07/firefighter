all:
	g++ main.cpp `sdl-config --cflags` `sdl-config --libs` -lSDL_ttf -o firefighter -g
run:
	./firefighter
clean:
	rm *.o
	rm firefighter
