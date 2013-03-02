all:
	gcc main.c `sdl-config --cflags` `sdl-config --libs` -o firefighter -g
run:
	./firefighter
clean:
	rm *.o
	rm firefighter
