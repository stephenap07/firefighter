#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#include "SDL/SDL.h"


#include "imgui.h"


/* SDL DEFINTITIONS */
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define SCREEN_DEPTH  32


/* STATES */
#define BURNING       0xff0000
#define PROTECTED     0x00ff00
#define FIREFIGHTER   0x0000ff
#define UNPROTECTED   0xffff00


/* Variables to change firefigher behavior */

int total_firefighers     = 1;
int total_fires           = 10;
int initial_fire_steps    = 1;
int milliseconds_per_turn = 100;

typedef Uint32 state;
typedef void (*functionDef)(int);

state fires[SCREEN_WIDTH][SCREEN_HEIGHT];
state firefighers[SCREEN_WIDTH][SCREEN_HEIGHT];
state buffer[SCREEN_WIDTH][SCREEN_HEIGHT];


void putPixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
void updateScreen(SDL_Surface *surface, Uint32 buffer[][SCREEN_HEIGHT]);
void spreadState(state buff[][SCREEN_HEIGHT], int x, int y, state st);


void initStates();
void initFire();
void updateFire();
void updateFirefighter(functionDef);
void preciseLanding(int total);

int pollEvent(SDL_Event event);
void renderUI(SDL_Surface *);


int main(int argc, char *argv[]) {
  /* Initialize SDL */
  SDL_Init(SDL_INIT_VIDEO);
  atexit(SDL_Quit);

  /* Initialize vertex states */
  initStates();
  initFire();
  int fire_steps = initial_fire_steps;

  /* Set Caption */
  SDL_WM_SetCaption("Firefighter Problem", NULL);

  /* Initialize the screen & window */
  gScreen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH, SDL_SWSURFACE);

  // If we fail, return error.
  if (gScreen == NULL) 
  {
    fprintf(stderr, "Unable to set up video: %s\n", SDL_GetError());
    exit(1);
  }

  // Enable keyboard repeat to make sliders more tolerable
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
  // Enable keyboard UNICODE processing for the text field.
  SDL_EnableUNICODE(1);  

 
  SDL_Surface *temp = SDL_LoadBMP("font14x24.bmp");
  gFont = SDL_ConvertSurface(temp, gScreen->format, SDL_SWSURFACE);
  SDL_FreeSurface(temp);


  SDL_Flip(gScreen);

  bool quit = false;
  bool done = false;
  SDL_Event event;

  Uint32 ticks = SDL_GetTicks();
  Uint32 current_ticks = 0;

  while(!quit) {
    renderUI(gScreen);
    int poll_result = pollEvent(event);

    if(poll_result >= 0) {
      return poll_result;
    }

    if ((ticks - current_ticks) >= milliseconds_per_turn) {
      current_ticks = SDL_GetTicks();

      if (!done) {
        if( SDL_MUSTLOCK(gScreen) )
        {
          SDL_LockSurface(gScreen);
        }

        updateFire();

        if (fire_steps <= 0) {
          updateFirefighter(&preciseLanding);
        }
        else {
          fire_steps--;
        }

        int x = 0;
        int y = 0;
        done = true;

        for(x = 0; x < SCREEN_WIDTH; x++) {
          for(y = 0; y < SCREEN_HEIGHT; y++) {
            if(firefighers[x][y] == FIREFIGHTER ||
               firefighers[x][y] == PROTECTED) {
              buffer[x][y] = firefighers[x][y];
            }
            if(fires[x][y] == BURNING &&
               buffer[x][y] != FIREFIGHTER &&
               buffer[x][y] != PROTECTED &&
               buffer[x][y] != BURNING) {
              buffer[x][y] = BURNING;
              done = false;
            }
          }
        }
        
        if (done) {

          for(x = 0; x < SCREEN_WIDTH; x++) {
            for(y = 0; y < SCREEN_HEIGHT; y++) {
              if (buffer[x][y] == UNPROTECTED) {
                buffer[x][y] = PROTECTED;
              }
            }
          }

        }

        updateScreen(gScreen, buffer);

        if(SDL_MUSTLOCK(gScreen)) SDL_UnlockSurface(gScreen);

        SDL_Flip(gScreen);
      }
    }
    else {
      ticks = SDL_GetTicks();
    }
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
  int i = 0;
  for (i=0; i < total_fires; i++) {
    srand(time(NULL) + i);

    int x = rand() % SCREEN_WIDTH;
    int y = rand() % SCREEN_HEIGHT;

    fires[x][y] = BURNING;
  }
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

}


/**
 * Loop through buffer, find fire, apply algorithm
 *
 * Algorithms include:
 *  - Surrounding fire with laser precision
 *  - Moving firefighter from a randomly selected home base
 *    to the fire in question
 *  - Accomodate for multiple fires
 */
void updateFirefighter(functionDef algorithm) {
  (*algorithm)(total_firefighers);
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

// AXIOMS
const int PRECISE_AXIOMS[4][4] = {
  {UNPROTECTED, UNPROTECTED, UNPROTECTED, BURNING},
  {UNPROTECTED, UNPROTECTED, BURNING,     UNPROTECTED},
  {UNPROTECTED, BURNING,     UNPROTECTED, UNPROTECTED},
  {BURNING,     UNPROTECTED, UNPROTECTED, UNPROTECTED},
};

/**
 * The algorithms
 */
void preciseLanding(int total) {
  int x = 0, y = 0;

  for(x = 0; x < SCREEN_WIDTH; x++) {
    for(y = 0; y < SCREEN_HEIGHT; y++) {

      if (total <= 0) {
        return;
      }

      if ((x+1) > (SCREEN_WIDTH-1)) { continue; }
      if ((y+1) > (SCREEN_HEIGHT-1)) { continue; }

      int radius[4] = {
        buffer[x][y], buffer[x+1][y],
        buffer[x][y+1], buffer[x+1][y+1]
      };
      
      int i = 0;

      for(i = 0; i < 4; i++) {
        if (radius[0] == PRECISE_AXIOMS[i][0] && 
            radius[1] == PRECISE_AXIOMS[i][1] && 
            radius[2] == PRECISE_AXIOMS[i][2] && 
            radius[3] == PRECISE_AXIOMS[i][3])
        {
          int x1 = 0;
          int y1 = 0;

          switch(i) {
            case 0:
              x1 = 0;
              y1 = 0;
              break;
            case 1:
              x1 = 1;
              y1 = 0;
              break;
            case 2: 
              x1 = 0;
              y1 = 1;
              break;
            case 3:
              x1 = 1;
              y1 = 1;
              break;
          };

          firefighers[x + x1][y + y1] = FIREFIGHTER;
          spreadState(firefighers, x + x1, y + y1, PROTECTED);
          total--;
          break;
        }
      }

    }
  }

}

/**
 * Set pixel for given surface
 *
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

/**
 * IMGUI Stuff
 */

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

// Rendering function
void renderUI(SDL_Surface *surface)
{   
  static int bgcolor = 0x77;
  static char sometext[80] = "Some text";

  imgui_prepare();

  button(GEN_ID,50,50);
  
  imgui_finish();
  
  // update the screen
  SDL_UpdateRect(gScreen, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);    
}



