///////////////////////////////////////////////////////////////////////////////
//!
//! @file
//!    endpoint.hpp
//!
//! @brief
//!    IPv4 address and port end-point wrapper.
//!
//! @author
//!    Copyright (c) 2023, John Ryland,
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
//!    Wraps a 32-bit ip address and 16-bit port as an end-point object.

#pragma once

///////////////////////////////////////////////////////////////////////////////
// Includes

#include "endian.hpp"
#include "string_utilities.hpp"

enum ProtocolFamily
{
  AF_INET
};

/// @brief Portable socket address for using with native socket APIs
struct SocketAddress
{
  uint16_t   m_family;  ///< Protocol family.
  uint16_be  m_port;    ///< Port number.
  uint32_be  m_address; ///< IPv4 address.
  uint8_t    m_zero[8]; ///< Padding.
};

/// @brief Encapsulation of an IPv4 address and port number representing the
///        source or destination of a socket connection.
class EndPoint
{
public:
  /// @brief EndPoint constructor from octets of the IP address and port number
  /// @param a_octet1 In a numeric IP address, this is the aaa in aaa.bbb.ccc.ddd.
  /// @param a_octet2 In a numeric IP address, this is the bbb in aaa.bbb.ccc.ddd.
  /// @param a_octet3 In a numeric IP address, this is the ccc in aaa.bbb.ccc.ddd.
  /// @param a_octet4 In a numeric IP address, this is the ddd in aaa.bbb.ccc.ddd.
  /// @param a_port A 16-bit port number.
  /// @note Port numbers below 1024 are priviledged and may require administrator permissions.
  EndPoint(uint8_t a_octet1, uint8_t a_octet2, uint8_t a_octet3, uint8_t a_octet4, uint16_t a_port)
    : m_ipaddr((a_octet1 << 24U) | (a_octet2 << 16U) | (a_octet3 << 8U) | a_octet4)
    , m_port(a_port)
  {
  }

  /// @brief Constructs a SocketAddress which is suitable for use with native socket APIs.
  /// @return Returns the SocketAddress object.
  SocketAddress socketAddress() const
  {
    return SocketAddress{ AF_INET, m_port, m_ipaddr };
  }

private:
  uint32_t  m_ipaddr;
  uint16_t  m_port;
};

/// @brief Converts a SocketAddress in to a fixed sized string suitable for display.
/// @param a_address is the object to convert to a string.
/// @return Returns a String<N> string in the form 'aaa.bbb.ccc.ddd:65536'.
inline auto to_string(const SocketAddress& a_address)
{
  uint32_t addr = a_address.m_address;
  return  to_string(uint8_t(addr >> 24U)) + to_string(".") + to_string(uint8_t(addr >> 16U)) + to_string(".")
        + to_string(uint8_t(addr >>  8U)) + to_string(".") + to_string(uint8_t(addr >>  0U))
        + to_string(":") + to_string(a_address.m_port);
}

/// @brief Converts an EndPoint in to a fixed sized string suitable for display.
/// @param a_endPoint is the object to convert to a string.
/// @return Returns a String<N> string in the form 'aaa.bbb.ccc.ddd:65536'.
inline auto to_string(const EndPoint& a_endPoint)
{
  return to_string(a_endPoint.socketAddress());
}
