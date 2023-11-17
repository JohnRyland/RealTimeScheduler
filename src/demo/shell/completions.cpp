//
//  completions.cpp
//  DescriptionHere
//
//  Created by John Ryland (jryland@xiaofrog.com) on 19/11/2017.
//  Copyright 2017 InvertedLogic. All rights reserved.
//
#include "utils.h"
#include "completions.h"


ReadLineWithHistory::ReadLineWithHistory(EditTextInterface& text)
  : Parent(text)
{
}


void ReadLineWithHistory::prepare()
{
  Parent::prepare();
  historyI = lineHistoryBuffer.size();
  lineHistoryBuffer.push_back("");
}


bool ReadLineWithHistory::processKeyPress(unsigned int ch)
{
  switch (ch) {
    case KeyProviderInterface::UP:
      if (historyI >= 0)
        lineHistoryBuffer[historyI] = text.getString();
      if (historyI > 0) {
        historyI--;
        text.fromString(lineHistoryBuffer[historyI]);
      } else {
        historyI = -1;
        text.fromString("");
      }
      break;
    case KeyProviderInterface::DOWN:
      if (historyI >= 0)
        lineHistoryBuffer[historyI] = text.getString();
      if (historyI == -1 || historyI < lineHistoryBuffer.size()-1) {
        historyI++;
        text.fromString(lineHistoryBuffer[historyI]);
      }
      break;
    default:
      if (Parent::processKeyPress(ch)) {
        lineHistoryBuffer.back() = text.getString();
        return true;
      }
      break;
  }
  return false;
}


CommandTabCompletions::CommandTabCompletions(const std::vector<std::string>& cmdList)
  : commandList(cmdList)
{
}


std::vector<CompletionsProviderInterface::Completion> 
  CommandTabCompletions::getCompletions(const std::string& contextBeforeText, const std::string& partialText)
{
  std::vector<Completion> matches;
  for (int i = 0; i < commandList.size(); i++)
  {
    std::string cmd = commandList[i];
    if (strncmp(partialText.c_str(), cmd.c_str(), partialText.size()) == 0)
    {
      Completion completion;
      completion.shortDisplayText = cmd;
      completion.completionText = cmd;
      matches.push_back(completion);
    }
  }
  return matches;
}


void CommandTabCompletions::showPossibleCompletions(const std::vector<Completion>& completions)
{
  printf("\n");
  for (int i = 0; i < completions.size(); i++)
    printf("%s ", completions[i].shortDisplayText.c_str());
  printf("\n");
}


FileTabCompletions::FileTabCompletions(FileSystem& fileSys)
  : fs(fileSys)
{
}


std::vector<CompletionsProviderInterface::Completion>
  FileTabCompletions::getCompletions(const std::string& contextBeforeText, const std::string& partialText)
{
  // TODO: search PATH for exe names to complete on
  //       perhaps some context sensitive smarts on completion context
  //bool incPathSearch = contextBeforeText.empty();
  //bool incPathSearch = contextBeforeText.size() == partialText.size();
  std::vector<Completion> matches;
  DirectoryEntrySet entries;
  std::string path = fs.dirname(partialText) + "/";
  fs.getDirectoryListing(entries, path);
  if (path == "./" && (partialText.size() < 1 || partialText[0] != '.'))
    path = "";
  for (int i = 0; i < entries.entries.size(); i++)
  {
    std::string basename = entries.entries[i].basename;
    bool isDirectory = entries.entries[i].type == DirectoryEntrySet::DirectoryEntryMember::Directory;
    if (strncmp(partialText.c_str(), (path + basename).c_str(), partialText.size()) == 0)
    {
      Completion completion;
      completion.shortDisplayText = basename + (isDirectory ? "/" : " ");
      completion.completionText = path + basename + (isDirectory ? "/" : " ");
      matches.push_back(completion);
    }
  }
  return matches;
}


void FileTabCompletions::showPossibleCompletions(const std::vector<Completion>& completions)
{
  printf("\n");
  for (int i = 0; i < completions.size(); i++)
    printf("%s ", completions[i].shortDisplayText.c_str());
  printf("\n");
}


ReadLineWithCompletions::ReadLineWithCompletions(EditTextInterface& text, CompletionsProviderInterface& cp_arg0, CompletionsProviderInterface& cp)
  : Parent(text)
  , commandCompletions(cp_arg0)
  , completions(cp)
{
}


void ReadLineWithCompletions::prepare()
{
  Parent::prepare();
  lastCh = 0;
}


bool ReadLineWithCompletions::processKeyPress(unsigned int ch)
{
  if (ch == KeyProviderInterface::TAB) {
    std::string curText = text.getStringUpToCursor();
    std::vector<std::string> args = strSplit(curText.c_str(), ' ');
    std::string lstArg = "";
    if (args.size() && curText[curText.size()-1] != ' ')
      lstArg = args.back();

    //printf("lst arg: -%s-\n", lstArg.c_str());
    std::vector<CompletionsProviderInterface::Completion> matches;

    if (args.size() == 1)
      matches = commandCompletions.getCompletions(curText, lstArg);
    else
      matches = completions.getCompletions(curText, lstArg);

    int commonLength = 0;
    if (matches.size() >= 1) {
      bool done = false;
      // Find the common sub-expression in the matches
      while (!done) {
        int offset = lstArg.size() + commonLength;
        if (matches[0].completionText.size() <= offset) {
          commonLength++;
          break;
        }
        char c = matches[0].completionText.c_str()[offset];
        for (int i = 1; i < matches.size(); i++) {
          if (matches[i].completionText.size() <= offset) {
            done = true;
            break;
          }
          if (c != matches[i].completionText.c_str()[offset]) {
            done = true;
            break;
          }
        }
        commonLength++;
      }
    }

    if (commonLength)
      commonLength--;

    // if there is a common-sub-expression to expand with the tab completion, output it
    if (commonLength) {
      // insert the TAB completion in to the input buffer
      const char* s = matches[0].completionText.c_str() + lstArg.size();
      for (int i = 0; i < commonLength; i++) {
        text.insertChar(*s);
        s++;
      }
      lastCh = ' ';
      return false; // prevent if TAB pressed again that it doesn't do double-TAB action
    }

    // If press TAB twice, list the options
    if (matches.size() > 1 && lastCh == KeyProviderInterface::TAB) {
      completions.showPossibleCompletions(matches);
    }
    lastCh = ch;
    return false;
  }
  lastCh = ch;
  return Parent::processKeyPress(ch);
}


