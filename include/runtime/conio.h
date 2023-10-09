/*
  Real-time Scheduler
  Copyright (c) 2022, John Ryland
  All rights reserved.
*/

#pragma once

// 43x50 character screen mode
#define C4350  3

// TC/DOS conio library functions
extern "C"
{
  void clrscr();
  void gotoxy(unsigned,unsigned);
  void textmode(int);
  int getch();
  void putch(char);
  bool kbhit();
  void puts2(const char*);
};

