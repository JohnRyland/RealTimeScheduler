//
//  keyboard.cpp
//  DescriptionHere
//
//  Created by John Ryland (jryland@xiaofrog.com) on 19/11/2017.
//  Copyright 2017 InvertedLogic. All rights reserved.
//
#include "keyboard.h"

// UNIX
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/select.h>


struct RawKeyboard::Data
{
  enum SpecialKeyCodes
  {
    ESCAPE_CODE_SEQ1 = 27,
    ESCAPE_CODE_SEQ2 = 91
  };
  int keyFd;
  struct termios savedFlags;
  bool read(unsigned int& ch, bool wait = true);
};


bool RawKeyboard::Data::read(unsigned int& ch, bool wait)
{
  ch = 0;
  if (wait) {
    fd_set rdSet;
    FD_ZERO(&rdSet);
    FD_SET(keyFd, &rdSet);
    select(keyFd + 1, &rdSet, NULL, NULL, NULL);
  }
  return ::read(keyFd, &ch, 1) != -1;
}


RawKeyboard::RawKeyboard()
  : data(new Data)
{
  data->keyFd = open("/dev/tty", O_RDONLY | O_NONBLOCK);
  tcgetattr(data->keyFd, &data->savedFlags);    // save state
  struct termios rawFlags = data->savedFlags;
  cfmakeraw(&rawFlags);                         // uncook the input
  rawFlags.c_oflag = data->savedFlags.c_oflag;  // restore output flags (only want to uncook the input)
  tcsetattr(data->keyFd, TCSANOW, &rawFlags);
}


RawKeyboard::~RawKeyboard()
{
  tcsetattr(data->keyFd, TCSANOW, &data->savedFlags); // restore state
  close(data->keyFd);
  delete data;
}

  
unsigned int RawKeyboard::readKey()
{
  unsigned int ch;
  if (!data->read(ch))
    return INVALID;
  if (ch == RawKeyboard::Data::ESCAPE_CODE_SEQ1) {
    if (data->read(ch, false)) {
      if (ch == RawKeyboard::Data::ESCAPE_CODE_SEQ2) {
        data->read(ch);
        switch (ch) {
          case 'A': return UP;
          case 'B': return DOWN;
          case 'C': return RIGHT;
          case 'D': return LEFT;
          default:  return INVALID;
        }
      }
      return ch;
    } else {
      return ESC;
    }
  }
  return ch;
}


