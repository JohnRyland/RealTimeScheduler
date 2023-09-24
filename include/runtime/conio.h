/*
  Real-time Scheduler
  Copyright (c) 2022, John Ryland
  All rights reserved.
*/
#pragma once

// 43x50 character screen mode
#define C4350  3

extern unsigned random(unsigned x);
extern void clrscr();
extern void gotoxy(unsigned,unsigned);
extern void textmode(int);
extern int getch();
extern void putch(char);
extern bool kbhit();
extern void puts2(const char*);

