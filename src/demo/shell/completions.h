//
//  completions.h
//  DescriptionHere
//
//  Created by John Ryland (jryland@xiaofrog.com) on 19/11/2017.
//  Copyright 2017 InvertedLogic. All rights reserved.
//
#pragma once

#include "filesystem.h"
#include "lineinput.h"


class ReadLineWithHistory : public SimpleKeyProcessor
{
  typedef SimpleKeyProcessor Parent;
public:
  ReadLineWithHistory(EditTextInterface& t);
  virtual void prepare();
  virtual bool processKeyPress(unsigned int ch);

private:
  int historyI;
  std::vector<std::string> lineHistoryBuffer;
};


class CompletionsProviderInterface
{
public:
  struct Completion {
    std::string  shortDisplayText;
    std::string  completionText;
  };
  virtual ~CompletionsProviderInterface() {}
  virtual std::vector<Completion> getCompletions(const std::string& contextBeforeText, const std::string& partialText) = 0;
  virtual void showPossibleCompletions(const std::vector<Completion>& completions) = 0;
};


class CommandTabCompletions : public CompletionsProviderInterface
{
public:
  CommandTabCompletions(const std::vector<std::string>& commandList);
  std::vector<Completion> getCompletions(const std::string& contextBeforeText, const std::string& partialText);
  void showPossibleCompletions(const std::vector<Completion>& completions);
private:
  const std::vector<std::string>& commandList;
};


class FileTabCompletions : public CompletionsProviderInterface
{
public:
  FileTabCompletions(FileSystem& fileSys);
  std::vector<Completion> getCompletions(const std::string& contextBeforeText, const std::string& partialText);
  void showPossibleCompletions(const std::vector<Completion>& completions);
private:
  FileSystem& fs;
};


class ReadLineWithCompletions : public ReadLineWithHistory
{
  typedef ReadLineWithHistory Parent;
public:
  ReadLineWithCompletions(EditTextInterface& text, CompletionsProviderInterface& cp_arg0, CompletionsProviderInterface& cp);
  virtual void prepare();
  virtual bool processKeyPress(unsigned int ch);

private:
  int lastCh;
  CompletionsProviderInterface& commandCompletions;
  CompletionsProviderInterface& completions;
};


