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
#include <string.h>
#include <prinrval.h>
#include <prio.h>

#include <md/NSPR/SocketImpNSPR.h>
#include <md/NSPR/NSPRHelpers.h>

namespace vpr {

// ============================================================================
// Public methods.
// ============================================================================

// ----------------------------------------------------------------------------
// Open the socket.  This creates a new socket using the domain and type
// options set through member variables.
// ----------------------------------------------------------------------------
bool
SocketImpNSPR::open () {
    bool retval;
    PRFileDesc* new_sock = NULL;

    if(NULL != m_handle)
        return false;

    // NSPR has not concept of domain in socket creation
    // switch (m_local_addr.getFamily())

    switch (m_type) {
      case SocketTypes::STREAM:
        new_sock = PR_NewTCPSocket();
        break;
      case SocketTypes::DATAGRAM:
        new_sock = PR_NewUDPSocket();
        break;
      default:
        fprintf(stderr,
                "[vpr::SocketImpNSPR] ERROR: Unknown socket type value %d\n",
                m_local_addr.getFamily());
        break;
    }

    // If socket(2) failed, print an error message and return error status.
    if ( new_sock == NULL ) {
        fprintf(stderr,
                "[vpr::SocketImpNSPR] Could not create socket: \n");
        retval = false;
    }
    // Otherwise, return success.
    else {
        m_handle = new_sock;
        m_open = true;
        retval = true;
    }

    return retval;
}

// ----------------------------------------------------------------------------
// Bind this socket to the address in the host address member variable.
// ----------------------------------------------------------------------------
bool
SocketImpNSPR::bind () {
    bool retval;
    PRStatus status;

    // Bind the socket to the address in m_local_addr.
    status = PR_Bind(m_handle, m_local_addr.getPRNetAddr());

    // If that fails, print an error and return error status.
    if ( status == PR_FAILURE )
    {
       NSPR_PrintError("SocketImpNSPR::bind: Failed to bind.");
       retval = false;
    }
    // Otherwise, return success.
    else {
        m_bound = true;
        retval = true;
    }

    return retval;
}

// ----------------------------------------------------------------------------
// Connect the socket on the client side to the server side.  For a datagram
// socket, this makes the address given to the constructor the default
// destination for all packets.  For a stream socket, this has the effect of
// establishing a connection with the destination.
// ----------------------------------------------------------------------------
bool
SocketImpNSPR::connect () {
    bool retval;
    PRStatus status;

    if(m_bound)
    {
        vprDEBUG(0,0) << "SocketImpNSPR::connect: Socket alreay bound.  Can't connect" << vprDEBUG_FLUSH; 
        return false;
    }

    // Attempt to connect to the address in m_addr.
    status = PR_Connect(m_handle, m_remote_addr.getPRNetAddr(), PR_INTERVAL_NO_TIMEOUT);

    if ( status == PR_FAILURE )
    {
       NSPR_PrintError("SocketImpNSPR::connect: Failed to connect.");
       retval = false;
    }
    // Otherwise, return success.
    else
    {
        m_bound = true;
        retval = true;
    }

    return retval;
}

// ============================================================================
// Protected methods.
// ============================================================================

// ----------------------------------------------------------------------------
// Default constructor.  This just initializes member variables to reasonable
// defaults.
// ----------------------------------------------------------------------------
SocketImpNSPR::SocketImpNSPR (const SocketTypes::Type sock_type)
    : BlockIO(std::string("INADDR_ANY")), m_handle(NULL), m_type(sock_type),
      m_bound(false)
{
    /* Do nothing. */ ;
}

// ----------------------------------------------------------------------------
// Standard constructor.  This takes the given address (a string containing a
// hostname or an IP address), port, domain and type and stores the values in
// the member variables for use when opening the socket and performing
// communications.
// ----------------------------------------------------------------------------
SocketImpNSPR::SocketImpNSPR (const InetAddr& local_addr,
                            const InetAddr& remote_addr,
                            const SocketTypes::Type sock_type)
    : BlockIO(std::string("INADDR_ANY")), m_handle(NULL),
      m_local_addr(local_addr), m_remote_addr(remote_addr), m_type(sock_type), m_bound(false)
{;}

// ----------------------------------------------------------------------------
// Destructor.  This currently does nothing.
// ----------------------------------------------------------------------------
SocketImpNSPR::~SocketImpNSPR ()
{
    if ( m_handle != NULL ) {
       // PRClose(m_handle);     // XXX: Let it dangle
       m_handle = NULL;
    }
}

/**
 *
 */
bool
SocketImpNSPR::getOption (const SocketOptions::Types option,
                          struct SocketOptions::Data& data)
{
    bool retval;
    PRSocketOptionData opt_data;
    bool get_opt;

    // If this is true, PR_GetSocketOption() will be called.
    get_opt = true;

    switch (option) {
      case SocketOptions::Linger:
        opt_data.option = PR_SockOpt_Linger;
        break;
      case SocketOptions::ReuseAddr:
        opt_data.option = PR_SockOpt_Reuseaddr;
        break;
      case SocketOptions::KeepAlive:
        opt_data.option = PR_SockOpt_Keepalive;
        break;
      case SocketOptions::RecvBufferSize:
        opt_data.option = PR_SockOpt_RecvBufferSize;
        break;
      case SocketOptions::SendBufferSize:
        opt_data.option = PR_SockOpt_SendBufferSize;
        break;
      case SocketOptions::IpTimeToLive:
        opt_data.option = PR_SockOpt_IpTimeToLive;
        break;
      case SocketOptions::IpTypeOfService:
        opt_data.option = PR_SockOpt_IpTypeOfService;
        break;
      case SocketOptions::AddMember:
      case SocketOptions::DropMember:
        fprintf(stderr,
                "[vpr::SocketImpNSPR] Cannot get add/drop member socket option!\n");
        get_opt = false;
        break;
      case SocketOptions::McastInterface:
        opt_data.option = PR_SockOpt_McastInterface;
        break;
      case SocketOptions::McastTimeToLive:
        opt_data.option = PR_SockOpt_McastTimeToLive;
        break;
      case SocketOptions::McastLoopback:
        opt_data.option = PR_SockOpt_McastLoopback;
        break;
      case SocketOptions::NoDelay:
        opt_data.option = PR_SockOpt_NoDelay;
        break;
      case SocketOptions::MaxSegment:
        opt_data.option = PR_SockOpt_MaxSegment;
        break;
    }

    if ( get_opt ) {
        PRStatus status;

        status = PR_GetSocketOption(m_handle, &opt_data);

        if ( status == PR_SUCCESS ) {
            retval = false;

            // This extracts the information from the union passed to
            // PR_GetSocketOption() and puts it in our friendly
            // SocketOptions::Data object.
            switch (option) {
              case SocketOptions::Linger:
                data.linger.enabled = opt_data.value.linger.polarity;
                data.linger.seconds = PR_IntervalToSeconds(opt_data.value.linger.linger);
                break;
              case SocketOptions::ReuseAddr:
                data.reuse_addr = (opt_data.value.reuse_addr != 0 ? true
                                                                  : false);
                break;
              case SocketOptions::KeepAlive:
                data.keep_alive = (opt_data.value.keep_alive != 0 ? true
                                                                  : false);
                break;
              case SocketOptions::RecvBufferSize:
                data.recv_buffer_size = opt_data.value.recv_buffer_size;
                break;
              case SocketOptions::SendBufferSize:
                data.send_buffer_size = opt_data.value.send_buffer_size;
                break;
              case SocketOptions::IpTimeToLive:
                data.ip_ttl = opt_data.value.ip_ttl;
                break;
              case SocketOptions::IpTypeOfService:
             /*
                switch (opt_data.value.ip_tos) {
                  case IPTOS_LOWDELAY:
                    data.type_of_service = SocketOptions::LowDelay;
                    break;
                  case IPTOS_THROUGHPUT:
                    data.type_of_service = SocketOptions::Throughput;
                    break;
                  case IPTOS_RELIABILITY:
                    data.type_of_service = SocketOptions::Reliability;
                    break;
#ifdef IPTOS_LOWCOST
                  case IPTOS_LOWCOST:
                    data.type_of_service = SocketOptions::LowCost;
                    break;
#endif
                }
                */

                break;
              case SocketOptions::McastInterface:
//                data.mcast_if = InetAddr(opt_data.value.mcast_if);
                break;
              case SocketOptions::McastTimeToLive:
                data.mcast_ttl = opt_data.value.mcast_ttl;
                break;
              case SocketOptions::McastLoopback:
                data.mcast_loopback = opt_data.value.mcast_loopback;
                break;
              case SocketOptions::NoDelay:
                data.no_delay = (opt_data.value.no_delay != 0 ? true : false);
                break;
              case SocketOptions::MaxSegment:
                data.max_segment = opt_data.value.max_segment;
                break;
            }
        }
        else {
            retval = false;
            fprintf(stderr,
                    "[vpr::SocketImpNSPR] ERROR: Could not get socket option "
                    "for socket");
        }
    }
    else {
        retval = false;
    }

    return retval;
}

/**
 *
 */
bool
SocketImpNSPR::setOption (const SocketOptions::Types option,
                          const struct SocketOptions::Data& data)
{
    PRSocketOptionData opt_data;

    switch (option) {
      case SocketOptions::Linger:
        opt_data.option                = PR_SockOpt_Linger;
        opt_data.value.linger.polarity = data.linger.enabled;
        opt_data.value.linger.linger   = PR_SecondsToInterval(data.linger.seconds);
        break;
      case SocketOptions::ReuseAddr:
        opt_data.option           = PR_SockOpt_Reuseaddr;
        opt_data.value.reuse_addr = (data.reuse_addr ? PR_TRUE : PR_FALSE);
        break;
      case SocketOptions::KeepAlive:
        opt_data.option           = PR_SockOpt_Keepalive;
        opt_data.value.keep_alive = (data.keep_alive ? PR_TRUE : PR_FALSE);
        break;
      case SocketOptions::RecvBufferSize:
        opt_data.option                 = PR_SockOpt_RecvBufferSize;
        opt_data.value.recv_buffer_size = data.recv_buffer_size;
        break;
      case SocketOptions::SendBufferSize:
        opt_data.option                 = PR_SockOpt_SendBufferSize;
        opt_data.value.send_buffer_size = data.send_buffer_size;
        break;
      case SocketOptions::IpTimeToLive:
        opt_data.option       = PR_SockOpt_IpTimeToLive;
        opt_data.value.ip_ttl = data.ip_ttl;
        break;
      case SocketOptions::IpTypeOfService:
        opt_data.option = PR_SockOpt_IpTypeOfService;

        switch (data.type_of_service) {
          case SocketOptions::LowDelay:
//            opt_data.value.tos = ???;
            break;
          case SocketOptions::Throughput:
//            opt_data.value.tos = ???;
            break;
          case SocketOptions::Reliability:
//            opt_data.value.tos = ???;
            break;
          case SocketOptions::LowCost:
#ifdef IPTOS_LOWCOST
//            opt_data.value.tos = ???;
#else
            fprintf(stderr,
                    "[vpr::SocketImpNSPR] WARNING: This subsystem does not "
                    "support LowCost type of service!\n");
#endif
            break;
        }

        break;
      case SocketOptions::AddMember:
        opt_data.option = PR_SockOpt_AddMember;
//        opt_data.value.mcast_req.mcaddr = data.mcast_add_member.getMulticastAddr().getAddressValue();
//        opt_data.value.mcast_req.ifaddr = data.mcast_add_member.getInterfaceAddr().getAddressValue();
        break;
      case SocketOptions::DropMember:
        opt_data.option = PR_SockOpt_DropMember;
//        opt_data.value.mcast_req.mcaddr = data.mcast_drop_member.getMulticastAddr().getAddressValue();
//        opt_data.value.mcast_req.ifaddr = data.mcast_drop_member.getInterfaceAddr().getAddressValue();
        break;
      case SocketOptions::McastInterface:
        opt_data.option         = PR_SockOpt_McastInterface;
//        opt_data.value.mcast_if = data.mcast_if.getAddressValue();
        break;
      case SocketOptions::McastTimeToLive:
        opt_data.option          = PR_SockOpt_McastTimeToLive;
        opt_data.value.mcast_ttl = data.mcast_ttl;
        break;
      case SocketOptions::McastLoopback:
        opt_data.option               = PR_SockOpt_McastLoopback;
        opt_data.value.mcast_loopback = data.mcast_loopback;
        break;
      case SocketOptions::NoDelay:
        opt_data.option         = PR_SockOpt_NoDelay;
        opt_data.value.no_delay = (data.no_delay ? PR_TRUE : PR_FALSE);
        break;
      case SocketOptions::MaxSegment:
        opt_data.option            = PR_SockOpt_MaxSegment;
        opt_data.value.max_segment = data.max_segment;
        break;
    }

    vprASSERT((m_handle != NULL) && "Trying to set option on NULL handle");
    if(m_handle == NULL)
        return false;

    return (PR_SetSocketOption(m_handle, &opt_data) == PR_SUCCESS);
}

}; // End of vpr namespace
