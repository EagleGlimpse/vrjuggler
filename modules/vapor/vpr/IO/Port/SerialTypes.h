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

#ifndef _VPR_SERIAL_TYPES_H_
#define _VPR_SERIAL_TYPES_H_


namespace vpr {

// ----------------------------------------------------------------------------
//: Serial I/O types.
// ----------------------------------------------------------------------------
//!PUBLIC_API:
namespace SerialTypes {
    //: Possible update action values.
    enum UpdateActionOption {
        NOW,            // Perform change immediately
        DRAIN,          // Perform change after all output is transmitted
        FLUSH           // Perform chagne after all output is transmitted and
                        // discard all unread input
    };

    //: Possible flow action values.
    enum FlowActionOption {
        OUTPUT_OFF,     // Output is suspended
        OUTPUT_ON,      // Previously suspended output is restarted
        INPUT_OFF,      // Send a STOP character to the device
        INPUT_ON        // Send a START character to the device
    };

    //: Posslbie queue flushing options values.
    enum FlushQueueOption {
        INPUT_QUEUE,    // Flush data received but not read on input queue
        OUTPUT_QUEUE,   // Flush data written but not transmitted on output
                        // queue 
        IO_QUEUES       // Flush all data (combination of the above two)
    };
};

}; // End of namespace


#endif  /* _VPR_SERIAL_TYPES_H_ */
