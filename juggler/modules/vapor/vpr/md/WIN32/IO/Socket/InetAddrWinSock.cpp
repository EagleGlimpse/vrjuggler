/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000 by Iowa State University
 *
 * Original Authors:
 *   Allen Bierbaum, Christopher Just,
 *   Patrick Hartling, Kevin Meinert,
 *   Carolina Cruz-Neira, Albert Baker
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/

#include <vprConfig.h>

#include <stdio.h>

#include <md/WIN32/InetAddrWinSock.h>

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff   /* -1 return */
#endif


// ============================================================================
// External global variables.
// ============================================================================
extern int errno;


namespace vpr {

const InetAddrWinSock InetAddrWinSock::AnyAddr;      // Default constructor defaults to ANY addr

// ----------------------------------------------------------------------------
// Construct an address object using the given address.  It must be of the
// form <address>:<port> where <address> can be a hostname or a
// dotted-decimal IP address.
// ----------------------------------------------------------------------------
Status
InetAddrWinSock::setAddress (const std::string& address) {
    std::string::size_type pos;
    std::string host_addr, host_port;
    Uint16 port;
    Status retval;

    pos       = address.find(":");
    host_addr = address.substr(0, pos);
    host_port = address.substr(pos + 1);
    port      = (Uint16) atoi(host_port.c_str());

    retval = lookupAddress(host_addr);
    setPort(port);
    setFamily(SocketTypes::INET);

    return retval;
}

// ----------------------------------------------------------------------------
// Get the protocol family of this address structure.
// XXX: Are raw sockets allowed with WinSock, and if so, what is the PF value?
// ----------------------------------------------------------------------------
SocketTypes::Domain
InetAddrWinSock::getFamily (void) const {
    SocketTypes::Domain family;

    switch (m_addr.sin_family) {
#ifdef PF_LOCAL
      case PF_LOCAL:
#else
      case PF_UNIX:
#endif
        family = SocketTypes::LOCAL;
        break;
      case PF_INET:
        family = SocketTypes::INET;
        break;
#ifdef PF_INET6
      case PF_INET6:
        family = SocketTypes::INET6;
        break;
#endif
#if defined(PF_LINK) || defined(PF_RAW)
#ifdef PF_LINK
      case PF_LINK:
#else
      case PF_RAW:
#endif
        family = SocketTypes::LINK;
        break;
#endif
    }

    return family;
}

// ----------------------------------------------------------------------------
// Set the protocol family of this address structure.
// XXX: Are raw sockets allowed with WinSock, and if so, what is the PF value?
// ----------------------------------------------------------------------------
void
InetAddrWinSock::setFamily (const SocketTypes::Domain family) {
    switch (family) {
      case SocketTypes::LOCAL:
#ifdef PF_LOCAL
        m_addr.sin_family = PF_LOCAL;
#else
        m_addr.sin_family = PF_UNIX;
#endif
        break;
      case SocketTypes::INET:
        m_addr.sin_family = PF_INET;
        break;
      case SocketTypes::INET6:
#ifdef PF_INET6
        m_addr.sin_family = PF_INET6;
#else
        fprintf(stderr,
                "[vpr::InetAddrWinSock] WARNING: IPv6 not supported on this host!\n");
#endif
        break;
#if defined(PF_LINK) || defined(PF_RAW)
      case SocketTypes::LINK:
#ifdef PF_LINK
        m_addr.sin_family = PF_LINK;
#else
        m_addr.sin_family = PF_RAW;
#endif
        break;
#endif
      default:
        fprintf(stderr,
                "[vpr::InetAddrWinSock] ERROR: Unknown socket family value %d\n",
                family);
        break;
    }
}

// ----------------------------------------------------------------------------
// Get the IP address associated with this structure as a human-readable
// string.
// ----------------------------------------------------------------------------
std::string
InetAddrWinSock::getAddressString (void) const {
    char ip_addr[sizeof("255.255.255.255")];
    std::string ip_str;
    union {
        char c[sizeof(Uint32)];
        Uint32 value;
    } addr;

    addr.value = ntohl(m_addr.sin_addr.s_addr);

    // I don't know why Win32 has this stupid _snprintf() function.  Maybe I
    // am missing something because I'm not a Windows programmer...
    _snprintf(ip_addr, sizeof(ip_addr), "%u.%u.%u.%u", (Uint8) addr.c[0],
              (Uint8) addr.c[1], (Uint8) addr.c[2], (Uint8) addr.c[3]);
    ip_str = ip_addr;

    return ip_str;
}

// ----------------------------------------------------------------------------
// Look up the given address and store the value in m_addr.
// ----------------------------------------------------------------------------
Status
InetAddrWinSock::lookupAddress (const std::string& address) {
    Status retval;
    struct hostent* host_entry;

    // First, try looking the host up by name.
    host_entry = gethostbyname(address.c_str());

    // If that succeeded, put the result in m_remote_addr.
    if ( host_entry != NULL ) {
        copyAddressValue(host_entry->h_addr);
    }
    // If gethostbyname(3) failed, the address string may be an IP address.
    else {
        unsigned long addr;

        // Try looking it up with inet_addr(3).
        addr = inet_addr(address.c_str());

        // If the address string could not be found using inet_addr(3), then
        // return error status.
        if ( addr == INADDR_NONE ) {
            fprintf(stderr,
                    "[vpr::InetAddrWinSock] Could not find address for '%s': %s\n",
                    address.c_str(), strerror(errno));
            retval.setCode(Status::Failure);
        }
        // Otherwise, we found the integer address successfully.
        else {
            setAddressValue(addr);
        }
    }

    return retval;
}

}; // End of vpr namespace
