/*
  Device Tree Parser
  Copyright (C) 2023, by John Ryland
  All rights reserved
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define DISABLE_ENDIAN_TO_STRING
#include "../../include/common/endian.hpp"

// Read in complete file
uint8_t* slurp(const char* filename)
{
  FILE* file = fopen(filename, "rb");
  if (!file)
    return nullptr;
  fseek(file, 0L, SEEK_END);
  size_t siz = ftell(file);
  fseek(file, 0L, SEEK_SET);
  uint8_t* mem = (uint8_t*)malloc(siz);
  fread(mem, siz, 1, file);
  printf("got size %li bytes\n", siz);
  fclose(file);
  return mem;
}

// [1] 5.2
struct fdt_header
{
  uint32_be magic;
  uint32_be totalsize;
  uint32_be off_dt_struct;
  uint32_be off_dt_strings;
  uint32_be off_mem_rsvmap;
  uint32_be version;
  uint32_be last_comp_version;
  uint32_be boot_cpuid_phys;
  uint32_be size_dt_strings;
  uint32_be size_dt_struct;
};

// [1] 5.3.2
struct fdt_reserve_entry
{
  uint64_be address;
  uint64_be size;
};

// [1] 5.4.1
enum fdt_node_type
{
  FDT_BEGIN_NODE = 0x00000001,
  FDT_END_NODE   = 0x00000002,
  FDT_PROP       = 0x00000003,
  FDT_NOP        = 0x00000004,
  FDT_END        = 0x00000009,
};

// [1] 5.4.1
struct fdt_property
{
  uint32_be len;
  uint32_be nameoff;
};

// [2] 2.2.1.1
bool looks_like_string(const char* maybe_str, size_t len)
{
  for (int i = 0; i < len - 1; i++)
  {
    char ch = maybe_str[i];
    if ((ch >= '0' && ch <= '9') || ch == ' ' || ch == '_' || ch == '-' ||
        (ch >= 'a' && ch <= 'z') || ch == '+' || ch == '?' || ch == '#' ||
        (ch >= 'A' && ch <= 'Z') || ch == ',' || ch == '.')
      continue;
    return false;
  }
  // null check
  return maybe_str[len-1] == 0;
}

// [2] 2.3.1
bool is_compat_key(const char* key)
{
  return strcmp(key, "compatible") == 0;
}

int dtb_parse_node(int depth, const uint32_be* structs, const uint8_t* strings)
{
  uint8_t* name = (uint8_t*)structs;
  for (int x = 0; x < depth; ++x)
    printf("\t");
  printf("%s {\n", name);

  int i = 0;
  while (*name)
    ++i, ++name;
  ++i;
  i = (i + 3) / 4;

  for (; structs[i] != FDT_END_NODE; i++)
  {
    if (structs[i] == FDT_NOP)
      continue;
    if (structs[i] == FDT_BEGIN_NODE)
    {
      printf("\n");
      i += dtb_parse_node(depth + 1, structs + i + 1, strings) + 1;
    }
    if (structs[i] == FDT_PROP)
    {
      i++;
      uint32_t len = structs[i];
      i++;
      uint32_t nameoff = structs[i];
      for (int x = 0; x < depth; ++x)
        printf("\t");

      if (len == 0)
        printf("\t%s;\n", strings + nameoff);
      else if (looks_like_string((const char*)&structs[i+1], len))
        printf("\t%s = \"%s\";\n", strings + nameoff, (const char*)&structs[i+1]);
      else if (is_compat_key((const char*)(strings + nameoff)) || (len%4 != 0))
      {
        printf("\t%s = \"", strings + nameoff);
        for (int i2 = 0; i2 < len-1; ++i2)
          if (((const char *)&structs[i+1])[i2])
            printf("%c", ((const char *)&structs[i+1])[i2]);
          else
            printf("\\0");
        printf("\";\n");
      }
      else
      {
        printf("\t%s = <0x%02x", strings + nameoff, (int32_t)structs[i+1]);
        for (int i2 = 4; i2 < len; i2+=4)
          printf(" 0x%02x", (int32_t)structs[i+1+i2/4]);
        printf(">;\n");
      }
      i += (len + 3) / 4;
    }
  }

  for (int x = 0; x < depth; ++x)
    printf("\t");
  printf("};\n");

  return i;
}

int dtb_parse_data(const uint8_t* data)
{
  fdt_header* header = (fdt_header*)data;

  printf("magic:             0x%x\n", (uint32_t)header->magic);
  printf("totalsize:         %i\n",   (uint32_t)header->totalsize);
  printf("off_dt_struct:     0x%x\n", (uint32_t)header->off_dt_struct);
  printf("off_dt_strings:    0x%x\n", (uint32_t)header->off_dt_strings);
  printf("off_mem_rsvmap:    0x%x\n", (uint32_t)header->off_mem_rsvmap);
  printf("version:           %i\n",   (uint32_t)header->version);
  printf("last_comp_version: %i\n",   (uint32_t)header->last_comp_version);
  printf("boot_cpuid_phys:   %i\n",   (uint32_t)header->boot_cpuid_phys);
  printf("size_dt_strings:   0x%x\n", (uint32_t)header->size_dt_strings);
  printf("size_dt_struct:    0x%x\n", (uint32_t)header->size_dt_struct);

  printf("============\n");
  printf(" memory map\n");
  printf("============\n");
  const fdt_reserve_entry* mem_map = (fdt_reserve_entry*)(data + (uint32_t)header->off_mem_rsvmap);
  for (int i = 0; mem_map[i].address != 0 || mem_map[i].size != 0; ++i)
    printf(" addr: %llx, size: %llx   (Reserved)\n", (uint64_t)mem_map[i].address, (uint64_t)mem_map[i].size);
  printf("============\n");
  

  const uint8_t* strings = data + (uint32_t)header->off_dt_strings;
  const uint32_be* structs = (uint32_be*)(data + (uint32_t)header->off_dt_struct);

  printf("/"); 
  int i = dtb_parse_node(0, structs, strings);
  while (true)
  {
    ++i;
    if (structs[i] == FDT_NOP)
      continue;
    if (structs[i] == FDT_END)
      break;
    printf("unexpected data that wasn't parsed\n");
  }

  return 0;
}

int dtb_parse_file(const char* filename)
{
  uint8_t* data = slurp(filename);
  if (!data)
    return 1;
  int res = dtb_parse_data(data);
  free(data);
  return res;
}

int main(int argc, const char* argv[])
{
  if (argc <= 1)
  {
    printf("Usage:  %s <dtb-file>\n", argv[0]);
    return 1;
  }
  return dtb_parse_file(argv[1]);
}

// Reference:
//  [1]  https://devicetree-specification.readthedocs.io/en/stable/flattened-format.html
//  [2]  https://devicetree-specification.readthedocs.io/en/stable/devicetree-basics.html

