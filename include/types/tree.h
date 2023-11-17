/*
  Statically Initializable Tree
  Copyright (c) 2023, John Ryland
  All rights reserved.
*/

#pragma once

#include "debug_logger.h"
#include "types/integers.h"

// See pci_descriptions.h for an example usage.
//
// Data for the nodes is pre-allocated up to N nodes. This is however per tree_t type,
// so if use this with different T values or different N values, then more nodes are
// available, but this is not usually sharable between trees but does mean you can
// have multiple trees, and a decision about number of nodes in one tree doesn't affect
// another tree.
//
// It enables defining a tree structure that is constructed when the program loads.
// Each tree_t represents a node, and each node can have up to 256 child nodes.
// Those children are also nodes so it is recursive. Each child has assigned a 8-bit
// code, these must be in sorted order, however they can have gaps. A find function
// is provided to find a child with a given code (however the searching is non-recursive
// of the tree, the code only has to be unique with siblings of the same parent node).

template <typename T, size_t N>
class tree_t
{
public:
  tree_t(uint8_t code_ = 0, T data_ = 0)
    : _code(code_), _count(0), _node_idx(last_node_idx), _data(data_)
  {
  }

  template <size_t N2>
  tree_t(uint8_t code_, T data_, const tree_t<T,N> (&branches_)[N2])
    : _code(code_), _count(N2), _node_idx(last_node_idx), _data(data_)
  {
    size_t capacity = N - last_node_idx;
    size_t copyN = (capacity < N2) ? capacity : N2;
    for (size_t i = 0; i < copyN; i++)
      // make a copy of the temporary
      nodes[last_node_idx + i] = branches_[i];
    last_node_idx += copyN;
  }

  const tree_t<T,N>* find(uint8_t code_) const
  {
    // clang was having an issue with the binary find code, if other compilers have issues, can use linear instead
    return binary_find(code_, 0, _count);

    // Uncomment if want to use linear searching instead of binary searching
    // return linear_find(code_, 0, _count);
  }

  uint8_t code() const                { return _code; }
  uint8_t count() const               { return _count; }
  const char* data() const            { return _data; }
  const tree_t<T,N>* branches() const { return &nodes[_node_idx]; }

private:
  const tree_t<T,N>* linear_find(uint8_t code_, uint8_t i1, uint8_t i2) const
  {
    for (uint8_t i = i1; i < i2; i++)
      if (nodes[_node_idx + i]._code == code_)
        return &nodes[_node_idx + i];
    return nullptr;
  }

  // Was previously using recursion, however using clang to compile it, the code didn't work.
  // Switching the code to use iteration instead works well.
  const tree_t<T,N>* binary_find(uint8_t code_, uint8_t i1, uint8_t i2) const
  {
    uint16_t idx1 = _node_idx + i1;
    uint16_t idx2 = _node_idx + i2;
    // Only need to iterate 8 times for up to 256 entries to search
    for (int depth = 0; depth < 8; depth++)
    {
      uint16_t mid = (idx1 + idx2) >> 1;
      if (code_ == nodes[mid]._code)
        return &nodes[mid];
      if (code_ > nodes[mid]._code)
        idx1 = mid;
      else
        idx2 = mid;
    }
    return nullptr;
  }

  // 4 bytes, + sizeof(T)
  uint8_t _code;
  uint8_t _count;
  uint16_t _node_idx;
  T _data;

  static uint16_t last_node_idx;
  static tree_t<T,N> nodes[N];
};
