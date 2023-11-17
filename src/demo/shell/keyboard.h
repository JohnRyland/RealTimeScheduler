//
//  keyboard.h
//  DescriptionHere
//
//  Created by John Ryland (jryland@xiaofrog.com) on 19/11/2017.
//  Copyright 2017 InvertedLogic. All rights reserved.
//
#pragma once


class KeyProviderInterface
{
public:
  enum KeyCodes
  {
    INVALID          = -1,
    CTRL_C           = 3,
    TAB              = 9,
    ENTER            = '\r',
    BACKSPACE        = 127,
    UP               = 1024,
    DOWN             = 1025,
    LEFT             = 1026,
    RIGHT            = 1028,
    ESC              = 1029
  };

  virtual ~KeyProviderInterface() {}
  virtual unsigned int readKey() = 0;
};


// Platform specific implementation of the interface
// This particulat implementation reads from the terminal tty on MacOSX
class RawKeyboard : public KeyProviderInterface
{
public:
  RawKeyboard();
  ~RawKeyboard();

  unsigned int readKey();

private:
  struct Data;
  Data* data;
};


