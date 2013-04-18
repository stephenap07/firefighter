#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"

#include "imgui.h"

/* SDL DEFINTITIONS */
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define SCREEN_DEPTH  32

int pollEvent(SDL_Event event);
void initCells(SDL_Surface *screen);

const unsigned int MAP_SIZE   = 20;
const unsigned int MAP_WIDTH  = SCREEN_WIDTH/MAP_SIZE;
const unsigned int MAP_HEIGHT = SCREEN_HEIGHT/MAP_SIZE;
const unsigned int COP        = 0xff0000ff;             // BLUE
const unsigned int ROBBER     = 0x0000ffff;             // RED

typedef struct entity {
  int x;
  int y;
  Uint32 color;
  bool (*pathfind)(struct entity *self, struct entity *target);
} entity_t;

typedef struct {
  int numCops; 
  int turnRate;
  int numTurns;
  int mapSize;
  int mapWidth;
  int mapHeight;
  bool caughtRobber;
  bool pause;

  entity_t *robber;
  entity_t *cops;

} state_t;

void initCells(SDL_Surface *screen);
void drawGrid(SDL_Surface *screen, Uint32 color);
void clearScreen(SDL_Surface *screen, Uint32 color);
void putPixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
void putCell(SDL_Surface *screen, int x, int y, Uint32 color);
void displayCells(SDL_Surface *screen, state_t *state);
bool copSimpleCatch(entity_t *self, entity_t *target);
bool robberSimpleRun(entity_t *self, entity_t *target);

// UI
void showStats(SDL_Surface *screen, TTF_Font *font, state_t *state);

int main(int argc, char *argv[]) {
  SDL_Surface *screen = gScreen;
  Uint8       *p;
  int         x = 10; //x coordinate of our pixel
  int         y = 20; //y coordinate of our pixel

  /* Initialize SDL */
  SDL_Init(SDL_INIT_VIDEO);
  if( TTF_Init() == -1) {
    fputs("Error loading font\n", stderr);
    exit(0);
  }

  TTF_Font * font = TTF_OpenFont("arial.ttf", 32);

  atexit(SDL_Quit);

  srand(time(NULL));

  /* Initialize the screen / window */
  screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH, SDL_SWSURFACE);

  entity_t robber = {rand() % MAP_WIDTH, rand() % MAP_HEIGHT, ROBBER};
  robber.pathfind = &robberSimpleRun;

  const int COP_COUNT= 5;
  entity_t cops[COP_COUNT];
  for(int i = 0; i < COP_COUNT; i++) {
    cops[i] = (entity_t){rand() % MAP_WIDTH, rand() % MAP_HEIGHT, COP};
  }

  for(int i = 0; i < COP_COUNT; i++) {
    cops[i].pathfind = &copSimpleCatch;
  }

  state_t state = {
    COP_COUNT, 300, 0, MAP_SIZE, SCREEN_WIDTH, SCREEN_HEIGHT, false, false, &robber, cops
  };

  SDL_Event event;

  bool quit = false;
  unsigned long ticks = 0;
  unsigned long delta = 0;
  unsigned long lastTick = SDL_GetTicks(); 

  while(!quit) {
    delta = SDL_GetTicks() - lastTick;
    lastTick = SDL_GetTicks();
    ticks += delta;

    int eventCode = pollEvent(event); 
    if (eventCode > -1) {
      quit = true;
    }

    if(state.caughtRobber) {
      state.pause = true;
    }

    clearScreen(screen, 0xffffffff);
    drawGrid(screen, 0x00000000);
    displayCells(screen, &state);

    if(!state.pause) {
      if(ticks >= state.turnRate) {
        robber.pathfind(&robber, state.cops);
        for(int i = 0; i < state.numCops; i++) {
          if(state.cops[i].pathfind(&state.cops[i], &robber)) {
            state.caughtRobber = true;
          }
        }
        state.numTurns++;
        ticks = 0;
      }
    }
    else {
      showStats(screen, font, &state); 
    }

    SDL_Flip(screen);
  }

}

bool copSimpleCatch(entity_t *self, entity_t *target) {
  if(self->x < target->x) {
    self->x++;
  }
  else if(self->x > target->x) {
    self->x--;
  }

  if(self->y < target->y) {
    self->y++;
  }
  else if(self->y > target->y) {
    self->y--;
  }

  if(self->x == target->x && self->y == target->y) {
    return true;
  }

  return false;
}

bool robberSimpleRun(entity_t *self, entity_t *target) {
  self->x += (rand() % 2) ? -1 : 1;
  self->y += (rand() % 2) ? -1 : 1;

  if(self->x < 0) {
    self->x = 0;
  }
  if(self->x > MAP_WIDTH - 1) {
    self->x = MAP_WIDTH - 1;
  }

  if(self->y < 0) {
    self->y = 0;
  }
  if(self->y > MAP_HEIGHT -  1) {
    self->y = MAP_HEIGHT - 1;
  }
  return true;
}

void displayCells(SDL_Surface *screen, state_t *state) {
  putCell(screen, state->robber->x, state->robber->y, state->robber->color);
  for (int i = 0; i < state->numCops; i++) {
    putCell(screen, state->cops[i].x, state->cops[i].y, state->cops[i].color);
  }
}

void putCell(SDL_Surface *screen, int x, int y, Uint32 color) {
  for(int j = 1; j < MAP_SIZE; j++) {
    for (int k = 1; k < MAP_SIZE; k++) {
      putPixel(screen, x * MAP_SIZE + k, y * MAP_SIZE + j, color);
    }     
  }
}

void clearScreen(SDL_Surface *screen, Uint32 color) {
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      putPixel(screen, x, y, color);
    }
  }
}

void drawGrid(SDL_Surface *screen, Uint32 color) {
  // Draw horizontal lines
  for(int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      putPixel(screen, x, MAP_SIZE*y, color);
    }     
  }

  for(int x = 0; x < MAP_WIDTH; x++) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
      putPixel(screen, x*MAP_SIZE, y, color);
    }     
  }
}

int pollEvent(SDL_Event event) {
  // Poll for events, and handle the ones we care about.
  while (SDL_PollEvent(&event)) 
  {
    switch (event.type) 
    {
      case SDL_MOUSEMOTION:
        // update mouse position
        uistate.mousex = event.motion.x;
        uistate.mousey = event.motion.y;
        break;
      case SDL_MOUSEBUTTONDOWN:
        // update button down state if left-clicking
        if (event.button.button == 1)
          uistate.mousedown = 1;
        break;
      case SDL_MOUSEBUTTONUP:
        // update button down state if left-clicking
        if (event.button.button == 1)
          uistate.mousedown = 0;
        break;
      case SDL_KEYDOWN:
        // If a key is pressed, report it to the widgets
        uistate.keyentered = event.key.keysym.sym;
        uistate.keymod = event.key.keysym.mod;
        // if key is ASCII, accept it as character input
        if ((event.key.keysym.unicode & 0xFF80) == 0)
          uistate.keychar = event.key.keysym.unicode & 0x7f;        
        break;
      case SDL_KEYUP:                  
        switch (event.key.keysym.sym)
        {
          case SDLK_ESCAPE:
            // If escape is pressed, return (and thus, quit)
            return 0;
        }
        break;
      case SDL_QUIT:
        return(0);
    }
  }

  return -1;
}

void showStats(SDL_Surface *screen, TTF_Font *font, state_t *state) {
  char numTurns[80];
  char numCops[80];

  sprintf(numCops, "Number of Cops: %d", state->numCops);
  sprintf(numTurns, "Number of Turns: %d", state->numTurns);

  char *messages[] = {numTurns, numCops};

  int i = 0;
  for(i = 0; i < 2; i++) {
    SDL_Surface *_message = TTF_RenderText_Solid(font, messages[i], gTextColor);
    apply_surface(SCREEN_WIDTH/2 - 130, SCREEN_HEIGHT/4 + 30*i, _message, screen);
    SDL_FreeSurface(_message);
  }
}

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
