/*
  Real-time Scheduler
  Copyright (c) 2022, John Ryland
  All rights reserved.
*/
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <sys/ioctl.h>
#include <termios.h>
#include "conio.h"

static void enable_raw_mode()
{
  termios term;
  tcgetattr(0, &term);
  term.c_lflag &= static_cast<unsigned>(~(ICANON | ECHO)); // Disable echo as well
  tcsetattr(0, TCSANOW, &term);
}

static void disable_raw_mode()
{
  termios term;
  tcgetattr(0, &term);
  term.c_lflag |= ICANON | ECHO;
  tcsetattr(0, TCSANOW, &term);
}

bool kbhit()
{
  //return false;
  enable_raw_mode();
  int byteswaiting;
  ioctl(0, FIONREAD, &byteswaiting);
  disable_raw_mode();
  return byteswaiting > 0;
}

unsigned random(unsigned x)
{
  return random() % x;
}

void clrscr()
{
  printf("\033c");
}

void gotoxy(unsigned x, unsigned y)
{
  printf("\033[%i;%iH", y, x);
}

void textmode(int)
{
  // does nothing
}

int getch()
{
  enable_raw_mode();
  int ch = getchar();
  disable_raw_mode();
  return ch;
}

void putch(char ch1)
{
  unsigned char ch = static_cast<unsigned char>(ch1);
  // These are each 3-bytes in utf-8 (but were a single byte in old DOS code-page / ROM font)
  const char* charLUT = "│┤╡╢╖╕╣║╗╝╜╛┐└┴┬├─┼╞╟╚╔╩╦╠═╬╧╨╤╥╙╘╒╓╫╪┘┌";
  if (ch >= 0xB3 && ch <= 0xDA)
  {
    ch -= 0xB3;
    char c1 = charLUT[ch*3+0];
    char c2 = charLUT[ch*3+1];
    char c3 = charLUT[ch*3+2];
    printf("%c%c%c", c1, c2, c3);
  }
  else if (ch == 0xDB)
  {
    printf("▇");
  }
  else
  {
    printf("%c", ch);
  }
}

void puts2(const char* str)
{
  while (*str)
  {
    putch(*str);
    ++str;
  }
  printf("\n");
}


