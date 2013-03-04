#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#include "SDL/SDL.h"

/* SDL DEFINTITIONS */
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define SCREEN_DEPTH  32

/* STATES */
#define BURNING       0xff0000
#define PROTECTED     0x00ff00
#define FIREFIGHTER   0x0000ff
#define UNPROTECTED   0xffff00

typedef Uint32 state;

state fires[SCREEN_WIDTH][SCREEN_HEIGHT];
state firefighers[SCREEN_WIDTH][SCREEN_HEIGHT];
state buffer[SCREEN_WIDTH][SCREEN_HEIGHT];

void putPixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
void updateScreen(SDL_Surface *surface, Uint32 buffer[][SCREEN_HEIGHT]);
void spreadState(state buff[][SCREEN_HEIGHT], int x, int y, state st);

void initStates();
void initFire();
void updateFire();

int main(int argc, char *argv[]) {
  SDL_Surface *screen;

  /* Initialize SDL */
  SDL_Init(SDL_INIT_VIDEO);

  /* Initialize vertex states */
  initStates();
  initFire();

  /* Set Caption */
  SDL_WM_SetCaption("Firefighter Problem", NULL);

  /* Initialize the screen & window */
  screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH, SDL_SWSURFACE);

  SDL_Flip(screen);

  bool quit = false;
  SDL_Event event;

  while(!quit) {
    while(SDL_PollEvent(&event))
    {  
      switch(event.type)
      {  
        case SDL_QUIT:
          quit = true;
          break;
      }
    }

    updateFire();
    updateScreen(screen, buffer);

    SDL_Flip(screen);
  }
}


/**
 * Update the screen to a match the buffer
 *
 * @param{surface} - surface drawn to
 * @param{buffer}  - pixel buffer
 */
void updateScreen(SDL_Surface *surface, Uint32 buffer[][SCREEN_HEIGHT])
{
  int x = 0 , y = 0;
  for(x = 0; x < surface->w; x++) {
    for(y = 0; y < surface->h; y++) {
      putPixel(surface, x, y, buffer[x][y]);
    }
  }
}


/**
 * Initialize pixel buffers
 *
 */
void initStates() {
  int x = 0;
  int y = 0;

  for(x = 0; x < SCREEN_WIDTH; x++) {
    for(y = 0; y < SCREEN_HEIGHT; y++) {
      fires[x][y] = 0;
      firefighers[x][y] = 0;
      buffer[x][y] = UNPROTECTED;
    }
  }
}


/**
 * Pick random location for a fire to start
 *
 */
void initFire() {
  srand(time(NULL));

  int x = rand() % SCREEN_WIDTH;
  int y = rand() % SCREEN_HEIGHT;

  fires[x][y] = BURNING;
}


/**
 * Expand fire and update pixel buffer
 *
 */
void updateFire() {
  int x = 0;
  int y = 0;

  for(x = 0; x < SCREEN_WIDTH; x++) {
    for(y = 0; y < SCREEN_HEIGHT; y++) {
      if(buffer[x][y] == BURNING) {
        spreadState(fires, x, y, BURNING);
      }
    }
  }

  for(x = 0; x < SCREEN_WIDTH; x++) {
    for(y = 0; y < SCREEN_HEIGHT; y++) {
      if(fires[x][y] == BURNING) {
        buffer[x][y] = BURNING;
      }
    }
  }

}


/**
 * Spread state to suround x and y
 *
 */
void spreadState(state buff[][SCREEN_HEIGHT], int x, int y, state st) {

  if(x + 1 < SCREEN_WIDTH) {
    buff[x + 1][y] = st;
  }
  if(x - 1 > 0) {
    buff[x - 1][y] = st;
  }
  if(y + 1 < SCREEN_HEIGHT) {
    buff[x][y + 1] = st;
  }
  if(y - 1 > 0) {
    buff[x][y - 1] = st;
  }
}


/**
 * Set pixel for given surface
 */
void putPixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
  int bpp = surface->format->BytesPerPixel;
  /* Here p is the address to the pixel we want to set */
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

  switch(bpp) {
    case 1:
      *p = pixel;
      break;

    case 2:
      *(Uint16 *)p = pixel;
      break;

    case 3:
      if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
        p[0] = (pixel >> 16) & 0xff;
        p[1] = (pixel >> 8) & 0xff;
        p[2] = pixel & 0xff;
      } else {
        p[0] = pixel & 0xff;
        p[1] = (pixel >> 8) & 0xff;
        p[2] = (pixel >> 16) & 0xff;
      }
      break;

    case 4:
      *(Uint32 *)p = pixel;
      break;
  }
}
