// If you're going to render widgets to the same
// UI from different source files, you can avoid
// ID collisions by defining IMGUI_SRC_ID before
// this define block:
#ifdef IMGUI_SRC_ID
#define GEN_ID ((IMGUI_SRC_ID) + (__LINE__))
#else
#define GEN_ID (__LINE__)
#endif

// Screen surface
SDL_Surface *gScreen;

// Font surface
SDL_Surface *gFont;

struct UIState
{
  int mousex;
  int mousey;
  int mousedown;

  int hotitem;
  int activeitem;

  int kbditem;
  int keyentered;
  int keymod;
  int keychar;
  
  int lastwidget;
} 
uistate = {0,0,0,0,0,0,0,0,0};

// Draw a single character.
// Characters are on top of each other in the font image, in ASCII order,
// so all this routine does is just set the coordinates for the character
// and use SDL to blit out.
void drawchar(char ch, int x, int y)
{
  SDL_Rect src, dst;
  src.w = 14;
  src.h = 24;
  src.x = 0;
  src.y = (ch - 32) * 24;
  dst.w = 14;
  dst.h = 24;
  dst.x = x;
  dst.y = y;
  SDL_BlitSurface(gFont, &src, gScreen, &dst);
}

// Draw the string. Characters are fixed width, so this is also
// deadly simple.
void drawstring(char * string, int x, int y)
{
  while (*string)
  {
    drawchar(*string,x,y);
    x += 14;
    string++;
  }
}


// Simplified interface to SDL's fillrect call
void drawrect(int x, int y, int w, int h, int color)
{
  SDL_Rect r;
  r.x = x;
  r.y = y;
  r.w = w;
  r.h = h;
  SDL_FillRect(gScreen, &r, color);
}


// Check whether current mouse position is within a rectangle
int regionhit(int x, int y, int w, int h)
{
  if (uistate.mousex < x ||
    uistate.mousey < y ||
    uistate.mousex >= x + w ||
    uistate.mousey >= y + h)
    return 0;
  return 1;
}

// Simple button IMGUI widget
int button(int id, int x, int y)
{
  // Check whether the button should be hot
  if (regionhit(x, y, 64, 48))
  {
    uistate.hotitem = id;
    if (uistate.activeitem == 0 && uistate.mousedown)
      uistate.activeitem = id;
  }

  // If no widget has keyboard focus, take it
  if (uistate.kbditem == 0)
    uistate.kbditem = id;

  // If we have keyboard focus, show it
  if (uistate.kbditem == id)
    drawrect(x-6, y-6, 84, 68, 0xff0000);

  // Render button 
  drawrect(x+8, y+8, 64, 48, 0);
  if (uistate.hotitem == id)
  {
    if (uistate.activeitem == id)
    {
      // Button is both 'hot' and 'active'
      drawrect(x+2, y+2, 64, 48, 0xffffff);
    }
    else
    {
      // Button is merely 'hot'
      drawrect(x, y, 64, 48, 0xffffff);
    }
  }
  else
  {
    // button is not hot, but it may be active    
    drawrect(x, y, 64, 48, 0xaaaaaa);
  }

  // If we have keyboard focus, we'll need to process the keys
  if (uistate.kbditem == id)
  {
    switch (uistate.keyentered)
    {
    case SDLK_TAB:
      // If tab is pressed, lose keyboard focus.
      // Next widget will grab the focus.
      uistate.kbditem = 0;
      // If shift was also pressed, we want to move focus
      // to the previous widget instead.
      if (uistate.keymod & KMOD_SHIFT)
        uistate.kbditem = uistate.lastwidget;
      // Also clear the key so that next widget
      // won't process it
      uistate.keyentered = 0;
      break;
    case SDLK_RETURN:
      // Had keyboard focus, received return,
      // so we'll act as if we were clicked.
      return 1;
    }
  }

  uistate.lastwidget = id;

  // If button is hot and active, but mouse button is not
  // down, the user must have clicked the button.
  if (uistate.mousedown == 0 && 
    uistate.hotitem == id && 
    uistate.activeitem == id)
    return 1;

  // Otherwise, no clicky.
  return 0;
}

// Simple scroll bar IMGUI widget
int slider(int id, int x, int y, int max, int &value)
{
  // Calculate mouse cursor's relative y offset
  int ypos = ((256 - 16) * value) / max;

  // Check for hotness
  if (regionhit(x+8, y+8, 16, 255))
  {
    uistate.hotitem = id;
    if (uistate.activeitem == 0 && uistate.mousedown)
      uistate.activeitem = id;
  }

  // If no widget has keyboard focus, take it
  if (uistate.kbditem == 0)
    uistate.kbditem = id;

  // If we have keyboard focus, show it
  if (uistate.kbditem == id)
    drawrect(x-4, y-4, 40, 280, 0xff0000);

  // Render the scrollbar
  drawrect(x, y, 32, 256+16, 0x777777);
  
  if (uistate.activeitem == id || uistate.hotitem == id)
  {
    drawrect(x+8, y+8 + ypos, 16, 16, 0xffffff);
  }
  else
  {
    drawrect(x+8, y+8 + ypos, 16, 16, 0xaaaaaa);
  }

  // If we have keyboard focus, we'll need to process the keys
  if (uistate.kbditem == id)
  {
    switch (uistate.keyentered)
    {
    case SDLK_TAB:
      // If tab is pressed, lose keyboard focus.
      // Next widget will grab the focus.
      uistate.kbditem = 0;
      // If shift was also pressed, we want to move focus
      // to the previous widget instead.
      if (uistate.keymod & KMOD_SHIFT)
        uistate.kbditem = uistate.lastwidget;
      // Also clear the key so that next widget
      // won't process it
      uistate.keyentered = 0;
      break;
    case SDLK_UP:
      // Slide slider up (if not at zero)
      if (value > 0)
      {
        value--;
        return 1;
      }
      break;
    case SDLK_DOWN:
      // Slide slider down (if not at max)
      if (value < max)
      {
        value++;
        return 1;
      }
      break;
    }
  }

  uistate.lastwidget = id;

  // Update widget value
  if (uistate.activeitem == id)
  {
    int mousepos = uistate.mousey - (y + 8);
    if (mousepos < 0) mousepos = 0;
    if (mousepos > 255) mousepos = 255;
    int v = (mousepos * max) / 255;
    if (v != value)
    {
      value = v;
      return 1;
    }
  }

  return 0;
}

int textfield(int id, int x, int y, char *buffer)
{
  int len = strlen(buffer);
  int changed = 0;

  // Check for hotness
  if (regionhit(x-4, y-4, 30*14+8, 24+8))
  {
    uistate.hotitem = id;
    if (uistate.activeitem == 0 && uistate.mousedown)
      uistate.activeitem = id;
  }

  // If no widget has keyboard focus, take it
  if (uistate.kbditem == 0)
    uistate.kbditem = id;

  // If we have keyboard focus, show it
  if (uistate.kbditem == id)
    drawrect(x-6, y-6, 30*14+12, 24+12, 0xff0000);

  // Render the text field
  if (uistate.activeitem == id || uistate.hotitem == id)
  {
    drawrect(x-4, y-4, 30*14+8, 24+8, 0xaaaaaa);
  }
  else
  {
    drawrect(x-4, y-4, 30*14+8, 24+8, 0x777777);
  }

  drawstring(buffer,x,y);

  // Render cursor if we have keyboard focus
  if (uistate.kbditem == id && (SDL_GetTicks() >> 8) & 1)
    drawstring("_",x + len * 14, y);

  // If we have keyboard focus, we'll need to process the keys
  if (uistate.kbditem == id)
  {
    switch (uistate.keyentered)
    {
    case SDLK_TAB:
      // If tab is pressed, lose keyboard focus.
      // Next widget will grab the focus.
      uistate.kbditem = 0;
      // If shift was also pressed, we want to move focus
      // to the previous widget instead.
      if (uistate.keymod & KMOD_SHIFT)
        uistate.kbditem = uistate.lastwidget;
      // Also clear the key so that next widget
      // won't process it
      uistate.keyentered = 0;
      break;
    case SDLK_BACKSPACE:
      if (len > 0)
      {
        len--;
        buffer[len] = 0;
        changed = 1;
      }
      break;      
    }
    if (uistate.keychar >= 32 && uistate.keychar < 127 && len < 30)
    {
      buffer[len] = uistate.keychar;
      len++;
      buffer[len] = 0;
      changed = 1;
    }
  }

  // If button is hot and active, but mouse button is not
  // down, the user must have clicked the widget; give it 
  // keyboard focus.
  if (uistate.mousedown == 0 && 
    uistate.hotitem == id && 
    uistate.activeitem == id)
    uistate.kbditem = id;

  uistate.lastwidget = id;

  return changed;
}

void imgui_prepare()
{
  uistate.hotitem = 0;
}

void imgui_finish()
{
  if (uistate.mousedown == 0)
  {
    uistate.activeitem = 0;
  }
  else
  {
    if (uistate.activeitem == 0)
      uistate.activeitem = -1;
  }
  // If no widget grabbed tab, clear focus
  if (uistate.keyentered == SDLK_TAB)
    uistate.kbditem = 0;
  // Clear the entered key
  uistate.keyentered = 0;  
  uistate.keychar = 0;
}


