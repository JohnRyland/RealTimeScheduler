//
//  lineinput.cpp
//  DescriptionHere
//
//  Created by John Ryland (jryland@xiaofrog.com) on 19/11/2017.
//  Copyright 2017 InvertedLogic. All rights reserved.
//
#include "lineinput.h"


LineBasedFile::LineBasedFile(const char* fileName)
{
  f = (fileName) ? fopen(fileName, "r") : NULL;
}


LineBasedFile::~LineBasedFile()
{
  if (f)
    fclose(f);
}


bool LineBasedFile::isOpen()
{
  return f != NULL;
}


// Includes the newline character
bool LineBasedFile::readLine(std::string& str)
{
  char line[1024];
  if (f && fgets(line, 1024, f) != 0) {
    str = line;
    return true;
  }
  return false;
}


ReadLineKeyboard::ReadLineKeyboard(KeyProviderInterface& kb, ProcessKeysInterface& pk)
  : rawKeys(kb)
  , processedKeys(pk)
{
}


bool ReadLineKeyboard::isOpen()
{
  return true;
}


// Includes the newline character
bool ReadLineKeyboard::readLine(std::string& retStr)
{
  processedKeys.prepare();
  unsigned int ch;
  while (!processedKeys.processKeyPress(ch = rawKeys.readKey())) {
    // do nothing
  }
  retStr = processedKeys.getProcessedText() + "\n";
  return (ch == KeyProviderInterface::CTRL_C) ? false : true;
}


bool SimpleKeyProcessor::processKeyPress(unsigned int ch)
{
  switch (ch) {
    case KeyProviderInterface::CTRL_C:
      printf("^C");
    case KeyProviderInterface::ENTER:
      printf("\n");
      return true;
    case KeyProviderInterface::BACKSPACE:
      text.deleteChar();
      break;
    case KeyProviderInterface::TAB:
    case KeyProviderInterface::UP:
    case KeyProviderInterface::DOWN:
      break;
    case KeyProviderInterface::RIGHT:
      text.cursorRight();
      break;
    case KeyProviderInterface::LEFT:
      text.cursorLeft();
      break;
    case KeyProviderInterface::ESC:
      printf("<ESC>\n");
      break;
    case (unsigned int)KeyProviderInterface::INVALID:
      printf("got invalid key code\n");
      break;
    default:
      if (isprint(ch)) {
        text.insertChar(ch);
      } else {
        printf("got ch: %i = %c\n", ch, ch); 
      }
      break;
  }
  return false;
}


void SimpleKeyProcessor::prepare()
{
  text.reset();
}


