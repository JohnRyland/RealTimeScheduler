//
//  edittext.cpp
//  DescriptionHere
//
//  Created by John Ryland (jryland@xiaofrog.com) on 19/11/2017.
//  Copyright 2017 InvertedLogic. All rights reserved.
//
#include "edittext.h"


void EditTextInterface::refreshLine(int newCursorI, int newStrLen)
{
  cursorI = newCursorI;
  strLen = newStrLen;
}


void EditTextInterface::reset()
{
  strLen = 0;
  cursorI = 0;
}


void EditTextInterface::cursorLeft()
{
  if (cursorI > 0)
    refreshLine(cursorI - 1, strLen);
}


void EditTextInterface::cursorRight()
{
  if (cursorI < strLen)
    refreshLine(cursorI + 1, strLen);
}


void EditTextInterface::insertChar(int ch)
{
  memmove(str + cursorI + 1, str + cursorI, strLen - cursorI);
  str[cursorI] = ch;
  refreshLine(cursorI + 1, strLen + 1);
}


void EditTextInterface::deleteChar()
{
  if (cursorI) {
    memcpy(str + cursorI - 1, str + cursorI, strLen - cursorI);
    refreshLine(cursorI - 1, strLen - 1);
  }
}


std::string EditTextInterface::getString()
{
  str[strLen] = 0;
  return str;
}


std::string EditTextInterface::getStringUpToCursor()
{
  int tmpCh = str[cursorI];
  str[cursorI] = 0;
  std::string s = str;
  str[cursorI] = tmpCh;
  return s;
}


void EditTextInterface::fromString(const std::string& s)
{
  memcpy(str, s.data(), s.size());
  refreshLine(s.size(), s.size());
}


void CommandLineText::refreshLine(int newCursorI, int newStrLen)
{
  clear();
  EditTextInterface::refreshLine(newCursorI, newStrLen);
  display();
  fflush(stdout);
}


void CommandLineText::clear()
{
  for (int i = 0; i < cursorI; i++)
    putchar('\b');
  for (int i = 0; i < strLen; i++)
    putchar(' ');
  for (int i = 0; i < strLen; i++)
    putchar('\b');
}


void CommandLineText::display()
{
  for (int i = 0; i < strLen; i++)
    putchar(str[i]);
  for (int i = 0; i < strLen - cursorI; i++)
    putchar('\b');
}


