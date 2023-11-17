///////////////////////////////////////////////////////////////////////////////
//!
//! @file
//!    dictionary.hpp
//!
//! @brief
//!    Dictionary implements a map from a string to an object.
//!
//! @author
//!    Copyright (c) 2017-2023, John Ryland,
//!    All rights reserved.
//!
//! License
//!    BSD-2-Clause License. See included LICENSE file for details.
//!    If LICENSE file is missing, see:
//!    https://opensource.org/licenses/BSD-2-Clause
//!
//!    Other licensing terms available.
//!    See LICENSE.Commercial for details.
//!
//! @details
//!    Similar to std::unordered_map<std::string,T>.

#pragma once

///////////////////////////////////////////////////////////////////////////////
// Includes

#include "string.hpp"
#include "utilities.hpp"
 
/// @todo Work in progress - not tested

// Modelled on same idea as String<N> class
template <typename T, size_t N, size_t MaxKeyLen>
class Dict
{
public:
    template <size_t Len>
    Dict<T, N+1, max(Len,MaxKeyLen)> append(const String<Len>& key, const T& item)
    {
        Dict<T, N+1, max(Len,MaxKeyLen)> ret;
        for (int i = 0; i < N; ++i)
            ret.keys[i] = keys[i]; // Copy?
        for (int i = 0; i < N; ++i)
            ret.items[i] = items[i];

        /// @todo insert in to a sorted position
        ret.keys[N] = key;
        ret.items[N] = item;
    }

    template <size_t X>
    const T &at() const
    {
        static_assert(N > X);
        return items[X];
    }

    const T &operator[](int x) const
    {
        assertion(N > x);
        return items[x];
    }

    template <size_t Len>
    const T &operator[](const String<Len>& key) const
    {
        /// @todo improve this
        for (int i = 0; i < N; i++)
            if (!strCmp(key, keys[i]))
                return items[i];
        static T ret;
        return ret;
    }

    int count() const
    {
        return N;
    }

protected:
    String<MaxKeyLen>  keys[N];
    T                  items[N];
};
