//
//  filesystem.cpp
//  DescriptionHere
//
//  Created by John Ryland (jryland@xiaofrog.com) on 19/11/2017.
//  Copyright 2017 InvertedLogic. All rights reserved.
//
#include "filesystem.h"

// UNIX
#include <unistd.h>
#include <dirent.h>

/*
FileSystem::File::File(std::string f, uint8_t t) : fileName(f), type(t)
{
}


std::string FileSystem::File::path()
{
  size_t pos = fileName.rfind(pathSeperator);
  return (pos == std::string::npos) ? std::string(".") : std::string(fileName, 0, pos);
}


bool FileSystem::File::isDirectory()
{
  return type & DT_DIR;
}
*/


static std::string getWorkingDir()
{
  char line[1024];
  return ::getcwd(line, 1024);
}


FileSystem::FileSystem()
{
  lastDir = curDir = getWorkingDir();
}


FileSystem::~FileSystem()
{
}


std::string FileSystem::simplifiedPath(const std::string& path)
{
  // TODO: implement simplifiedPath
  return path;
}


std::string FileSystem::dirname(const std::string& fileName)
{
  size_t pos = fileName.rfind(pathSeperator);
  return (pos == std::string::npos) ? std::string(".") : std::string(fileName, 0, pos);
}


std::string FileSystem::basename(const std::string& fileName)
{
  size_t pos = fileName.rfind(pathSeperator);
  return (pos == std::string::npos) ? fileName : std::string(fileName.c_str() + pos + 1);
}


std::string FileSystem::extension(const std::string& fileName)
{
  std::string base = basename(fileName);
  size_t pos = base.rfind('.');
  return (pos == std::string::npos) ? "" : std::string(base.c_str() + pos);
}


bool FileSystem::getDirectoryListing(DirectoryEntrySet& entries, std::string directory, bool hidden)
{
  DIR *dir = opendir(directory.c_str());
  if (!dir)
    return false;
  struct dirent *ent;
  entries.relativePath = directory;
  // entries.path = simplifiedPath(getCurrentDirectory() + directory); // TODO: test
  while ((ent = readdir(dir)) != NULL) {
    if (hidden || ent->d_name[0] != '.')
      entries.entries.push_back(DirectoryEntrySet::DirectoryEntryMember(ent->d_name, 
        (ent->d_type & DT_DIR) ? DirectoryEntrySet::DirectoryEntryMember::Directory : DirectoryEntrySet::DirectoryEntryMember::File
        ));
  }
  closedir(dir);
  return true;
}


DirectoryEntrySet FileSystem::currentDirectoryListing()
{
  DirectoryEntrySet ents;
  getDirectoryListing(ents, ".");
  return ents;
}


void FileSystem::popDirectory()
{
  changeDirectory(lastDir);
}


void FileSystem::changeDirectory(std::string path)
{
  lastDir = getWorkingDir();
  chdir(path.c_str());
  curDir = getWorkingDir();
}


std::string FileSystem::getCurrentDirectory()
{
  return curDir;
}


const char FileSystem::pathSeperator = '/';


