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

#ifndef _VPR_MCAST_REQ_H_
#define _VPR_MCAST_REQ_H_

#include <IO/Socket/InetAddr.h>


namespace vpr {

class McastReq {
public:
    /**
     * Default constructor.
     */
    McastReq (void)
        : m_mcast_addr(), m_if_addr()
    {
        /* Do nothing. */ ;
    }

    /**
     *
     */
    McastReq (const InetAddr& mcast_addr, const InetAddr& if_addr)
        : m_mcast_addr(mcast_addr), m_if_addr(if_addr)
    {
        /* Do nothing. */ ;
    }

    /**
     * ------------------------------------------------------------------------
     * Copy constructor.
     * ------------------------------------------------------------------------
     */
    McastReq (const McastReq& mcast_addr) {
        m_mcast_addr = mcast_addr.m_mcast_addr;
        m_if_addr    = mcast_addr.m_if_addr;
    }

    /**
     *
     */
    inline const InetAddr&
    getMulticastAddr (void) const {
        return m_mcast_addr;
    }

    /**
     *
     */
    inline const InetAddr&
    getInterfaceAddr (void) const {
        return m_if_addr;
    }

private:
    InetAddr m_mcast_addr;
    InetAddr m_if_addr;
};

}; // End of vpr namespace


#endif	/* _VPR_MCAST_REQ_H_ */
