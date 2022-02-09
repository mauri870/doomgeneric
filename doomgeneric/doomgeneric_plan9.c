//doomgeneric for plan9

#include <u.h>
#include <stdio.h>
#include <draw.h>
#include <event.h>
#include <keyboard.h>

#include "doomkeys.h"
#include "m_argv.h"
#include "doomgeneric.h"

#include <unistd.h>
#include <sys/time.h>

#include <stdbool.h>


#define KEYQUEUE_SIZE 16

char *buttons[] = {"exit", 0};
static Menu menu = {buttons};

static unsigned short s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int s_KeyQueueWriteIndex = 0;
static unsigned int s_KeyQueueReadIndex = 0;

unsigned short convertToDoomKey(int key)
{
    switch (key) {
    case '\n':
        key = KEY_ENTER;
        break;
    case Kesc:
        key = KEY_ESCAPE;
        break;
    case Kleft:
        key = KEY_LEFTARROW;
        break;
    case Kright:
        key = KEY_RIGHTARROW;
        break;
    case Kup:
        key = KEY_UPARROW;
        break;
    case Kdown:
        key = KEY_DOWNARROW;
        break;
    case Kctl:
        key = KEY_FIRE;
        break;
    case 20:
        key = KEY_USE;
        break;
    case Kshift:
        key = KEY_RSHIFT;
        break;
    case Kalt:
        key = KEY_RALT;
        break;
    default:
      if (key < 0x80)
          key = tolower(key);
      break;
    }

    return key;
}

static void addKeyToQueue(int keyCode)
{
    // TODO: is there a way to check if pressed in Plan9?
    bool pressed = true;
    unsigned short key = convertToDoomKey(keyCode);

    unsigned short keyData = (pressed << 8) | key;

    s_KeyQueue[s_KeyQueueWriteIndex] = keyData;
    s_KeyQueueWriteIndex++;
    s_KeyQueueWriteIndex %= KEYQUEUE_SIZE;
}

void writePixel(Image *dst, int x, int y, int color) {
	Rectangle r;

	r = dst->r;
  r.min.x += x;
  r.min.y += y;
  r.max.x = r.min.x + 1;
  r.max.y = r.min.y + 1;

  uchar alpha = (color >> 24) & 0xff;
  uchar red = (color >> 16) & 0xFF;
  uchar green = (color >> 8) & 0xFF;
  uchar blue = (color) & 0xFF;

  uchar bits[4];
  bits[3] = alpha;
  bits[2] = red;
  bits[1] = green;
  bits[0] = blue;
  loadimage(dst, r, bits, sizeof bits);
}

void redraw() {
  Image *frame;
  uchar *buf;
  uchar *d;
  int bufsiz;

	/* Back buffer */
	frame = allocimage(display, screen->r, screen->chan, RGBA32, DBlack);
	if(frame == nil)
		sysfatal("redraw frame allocimage: %r");

  for (int r = 0; r < DOOMGENERIC_RESY; ++r)
  {
      for (int c = 0; c < DOOMGENERIC_RESX; ++c)
      {
          unsigned int color = DG_ScreenBuffer[r * DOOMGENERIC_RESX + c];
          writePixel(frame, c, r, color);
      }
  }
  
  /* raw memory for the load/unload */
	bufsiz = bytesperline(frame->r, frame->depth) * Dy(frame->r);
	buf = malloc(bufsiz);

  /* move back buffer to front buffer */
	unloadimage(frame, frame->r, buf, bufsiz);
	loadimage(screen, frame->r, buf, bufsiz);
	
	flushimage(display, 1);
	free(buf);
	freeimage(frame);
}

void eresized(int new) {
	if (new && getwindow(display, Refnone) < 0)
		sysfatal("can't reattach to window");
  draw(screen, screen->r, display->black, nil, ZP);
  redraw();
} 

void DG_Init(){
  memset(s_KeyQueue, 0, KEYQUEUE_SIZE * sizeof(unsigned int));

  if (initdraw(nil, nil, "DOOM") < 0)
	  sysfatal("initdraw failed: %r");

  // TODO: uncomment this when the texture loading works otherwise
  // it hangs the rio window.
  einit(Emouse|Ekeyboard);
  eresized(0);
}

void maybeReadEvents() {
  if (!ecanread(Ekeyboard|Emouse))
    return;

  Event ev;
  switch (eread(Emouse|Ekeyboard, &ev)) {
		case Emouse:
			if (ev.mouse.buttons & 4 && emenuhit(3, &ev.mouse, &menu) == 0) {
					exit(0);
				}
			break;
		case Ekeyboard:
      addKeyToQueue(ev.kbdc);
			break;
	}
}

void DG_DrawFrame()
{
  maybeReadEvents();
  redraw();
}

void DG_SleepMs(uint32_t ms)
{
  sleep(ms);
}

uint32_t DG_GetTicksMs()
{
  struct timeval  tp;
  struct timezone tzp;

  gettimeofday(&tp, &tzp);

  return (tp.tv_sec * 1000) + (tp.tv_usec / 1000); /* return milliseconds */ 
}

int DG_GetKey(int* pressed, unsigned char* doomKey)
{
  if (s_KeyQueueReadIndex == s_KeyQueueWriteIndex)
	{
		//key queue is empty
		return 0;
	}

  unsigned short keyData = s_KeyQueue[s_KeyQueueReadIndex];
	s_KeyQueueReadIndex++;
	s_KeyQueueReadIndex %= KEYQUEUE_SIZE;

	*pressed = keyData >> 8;
	*doomKey = keyData & 0xFF;

  return 1;
}

void DG_SetWindowTitle(const char * title)
{
  (void) title;
}
