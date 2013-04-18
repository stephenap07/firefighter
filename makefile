all:
	g++ main.cpp `sdl-config --cflags` `sdl-config --libs` -lSDL_ttf -o copsnrobbers -O2
run:
	./copsnrobbers
clean:
	rm *.o
	rm copsnrobbers
