//
//  filesystem.h
//  DescriptionHere
//
//  Created by John Ryland (jryland@xiaofrog.com) on 19/11/2017.
//  Copyright 2017 InvertedLogic. All rights reserved.
//
#pragma once

#include <string>
#include <vector>
#include <stdint.h>


/*
// An in-memory representation of the directory tree
// However keeping in sync is an issue, so would need to constantly check
class DirectoryEntry
{
public:
  enum EntryType
  {
    Directory = 1,
    File      = 2
  };
  DirectoryEntry*                parent;
  std::string                    name;
  EntryType                      type;
  std::vector<DirectoryEntry*>   children;
};
*/


// Directory contents structure
class DirectoryEntrySet
{
public:
  class DirectoryEntryMember
  {
  public:
    enum EntryType
    {
      Directory = 1,
      File      = 2
    };
    DirectoryEntryMember(const std::string& s, EntryType t) : basename(s), type(t) {}
    std::string  basename;
    EntryType    type;
  };
  std::string path;
  std::string relativePath;
  std::vector<DirectoryEntryMember> entries;
};


// TODO: make filesystem in to an interface
class FileSystem
{
public:
  static const char pathSeperator;

/*
  class File
  {
  public:
    File(std::string f, uint8_t type = 0);
    //std::string basename();
    std::string path();
    bool isDirectory();
  private:
    std::string fileName;
    uint8_t type;
  };
*/

  FileSystem();
  ~FileSystem();

  /* static */ std::string simplifiedPath(const std::string& path);
  /* static */ std::string dirname(const std::string& fileName);
  /* static */ std::string basename(const std::string& fileName);
  /* static */ std::string extension(const std::string& fileName);

  bool getDirectoryListing(DirectoryEntrySet& entries, std::string directory, bool hidden = false);
  DirectoryEntrySet currentDirectoryListing();
  
  //bool getDirectoryListing(std::vector<File>& entries, std::string directory, bool hidden = false);
  //std::vector<File> currentDirectoryListing();

  void popDirectory();
  void changeDirectory(std::string path);
  std::string getCurrentDirectory();

private:
  std::string curDir, lastDir;
};


