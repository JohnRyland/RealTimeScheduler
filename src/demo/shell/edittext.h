//
//  edittext.h
//  DescriptionHere
//
//  Created by John Ryland (jryland@xiaofrog.com) on 19/11/2017.
//  Copyright 2017 InvertedLogic. All rights reserved.
//
#pragma once

#include <string>


class EditTextInterface
{
public:
  virtual ~EditTextInterface() {}
  virtual void refreshLine(int newCursorI, int newStrLen);
  void reset();
  void cursorLeft();
  void cursorRight();
  void insertChar(int ch);
  void deleteChar();
  std::string getString();
  std::string getStringUpToCursor();
  void fromString(const std::string& s);

protected:
  char str[1024];
  int strLen;
  int cursorI;
};


class CommandLineText : public EditTextInterface
{
public:
  void refreshLine(int newCursorI, int newStrLen);
  void clear();
  void display();
};

