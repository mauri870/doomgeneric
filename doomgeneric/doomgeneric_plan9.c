//doomgeneric for plan9

#include "doomkeys.h"
#include "m_argv.h"
#include "doomgeneric.h"

#include <stdio.h>
#include <unistd.h>

#include <stdbool.h>

void DG_Init(){
  printf("DG_Init\n");
}

void DG_DrawFrame()
{
  printf("DG_DrawFrame\n");
}

void DG_SleepMs(uint32_t ms)
{
  sleep(ms);
}

uint32_t DG_GetTicksMs()
{
  printf("DG_GetTicksMs\n");
}

int DG_GetKey(int* pressed, unsigned char* doomKey)
{
  (void) pressed;
  (void) doomKey;
  printf("DG_GetKey\n");
  return 0;
}

void DG_SetWindowTitle(const char * title)
{
  printf("DG_SetWindowTitle");
}
