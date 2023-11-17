//
//  utils.cpp
//  DescriptionHere
//
//  Created by John Ryland (jryland@xiaofrog.com) on 19/11/2017.
//  Copyright 2017 InvertedLogic. All rights reserved.
//
#include "utils.h"


std::vector<std::string> strSplit(const char* line, char seperator)
{
  std::vector<std::string> args;
  std::string str;
  while (*line)
  {
    if (*line == seperator)
    {
      args.push_back(str);
      str.clear();
      while (*line == seperator)
      {
        line++;
      }
    }
    else
    {
      str += *line;
      line++;
    }
  }
  if (!str.empty())
    args.push_back(str);
  return args;
}


