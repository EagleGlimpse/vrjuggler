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

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <VPR/IO/Port/vjSerialPort.h>
#include <VPR/IO/Port/vjSerialTypes.h>


int
main (int argc, char* argv[]) {
    vjSerialPort* port;

    port = new vjSerialPort(argv[1]);

    port->setOpenReadWrite();
    port->setOpenBlocking();

    if ( port->open() ) {
        char read_buffer[10], write_buffer[10];
        int val;
        ssize_t bytes;

        std::cerr << "Port opened\n";
        val = 1;

        port->setUpdateAction(vjSerialTypes::NOW);
        port->setCharacterSize(8);
        port->enableRead();

        for ( int i = 0; i < 10; i++ ) {
            bzero((void*) &write_buffer, sizeof(write_buffer));
            sprintf(write_buffer, "%d", val);
            bytes = port->write(write_buffer, strlen(write_buffer) + 1);
            std::cerr << "Wrote '" << write_buffer << "' (" << bytes
                      << " bytes)\n";

            bzero((void*) &read_buffer, sizeof(read_buffer));
            port->read(read_buffer, sizeof(read_buffer));
            std::cerr << "Read '" << read_buffer << "'\n";

            val = atoi(read_buffer);
            val++;
        }
    }

    return 0;
}