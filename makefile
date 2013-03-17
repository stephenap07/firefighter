all:
	g++ main.cpp `sdl-config --cflags` `sdl-config --libs` -lSDL_ttf -o firefighter -O2
run:
	./firefighter
clean:
	rm *.o
	rm firefighter
