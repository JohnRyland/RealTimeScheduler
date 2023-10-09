/*
  Symbol Map Generator
  Copyright (C) 2023, by John Ryland
  All rights reserved.

  Tool which processes input from nm output to
  produce a symbol map file that can be used
  by the kernel.
*/

#include <string>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include "../../include/types.h"

int main()
{
  std::vector<symbol_entry> offtab;
  std::string strtab;
  unsigned offset;
  while (std::cin >> std::hex >> offset)
  {
    std::string line;
    std::getline(std::cin, line);
    offtab.push_back(symbol_entry{offset, (uint32_t)strtab.size()});
    strtab += &line.c_str()[3]; // Skip past the symbol type character
  }

  symbol_table table = { 0xDDCC, (uint16_t)offtab.size() };
  fwrite(&table, sizeof(table), 1, stdout);
/*
  // Output magic header value
  uint8_t magic[2] = { 0xCC, 0xDD };
  fwrite(magic, sizeof(magic), 1, stdout);

  // Output symbol count
  uint16_t cnt = offtab.size();
  fwrite(&cnt, sizeof(cnt), 1, stdout);
*/

  // Output symbol entries (address + symbol string offset)
  for (auto& el : offtab)
  {
    //uint32_t vals[2] = { el.first, el.second };
    fwrite(&el, sizeof(el), 1, stdout);
  }

  // Output string table of the symbol names
  printf("%s", strtab.c_str());
}

