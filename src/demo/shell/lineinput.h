//
//  lineinput.h
//  DescriptionHere
//
//  Created by John Ryland (jryland@xiaofrog.com) on 19/11/2017.
//  Copyright 2017 InvertedLogic. All rights reserved.
//
#pragma once

#include "keyboard.h"
#include "edittext.h"


class LineInputInterface
{
public:
  virtual ~LineInputInterface() {}
  virtual bool isOpen() = 0;
  virtual bool readLine(std::string& str) = 0;
};


class LineBasedFile : public LineInputInterface
{
public:
  LineBasedFile(const char* fileName);
  ~LineBasedFile();
  bool isOpen();
  bool readLine(std::string& str);

private:
  FILE* f;
};


class ProcessKeysInterface
{
public:
  virtual ~ProcessKeysInterface() {}
  virtual void prepare() = 0;
  virtual bool processKeyPress(unsigned int ch) = 0;
  virtual std::string getProcessedText() = 0;
};


class SimpleKeyProcessor : public ProcessKeysInterface
{
public:
  SimpleKeyProcessor(EditTextInterface& t) : text(t) {}
  void prepare();
  bool processKeyPress(unsigned int ch);
  std::string getProcessedText() { return text.getString(); }
protected:
  EditTextInterface& text;
};


class ReadLineKeyboard : public LineInputInterface
{
public:
  ReadLineKeyboard(KeyProviderInterface& kb, ProcessKeysInterface& pk);
  bool isOpen();
  bool readLine(std::string& retStr);
protected:
  ProcessKeysInterface& processedKeys;
private:
  KeyProviderInterface& rawKeys;
};


