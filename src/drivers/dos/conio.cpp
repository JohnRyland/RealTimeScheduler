/*
  Real-time Scheduler
  Copyright (c) 2022, John Ryland
  All rights reserved.
*/
#include "conio.h"
#include <conio.h>

// TC/DOS conio library functions
bool kbhit();
int random(int x);
void clrscr();
void gotoxy(int x, int y);
void textmode(int);
int getch();
void putch(unsigned char ch);


void puts2(const char* str)
{
  while (*str)
  {
    putch(*str);
    ++str;
  }
  printf("\n");
}


