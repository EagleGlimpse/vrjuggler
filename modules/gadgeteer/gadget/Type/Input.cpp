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


#include <vjConfig.h>
#include <Input/vjInput/vjInput.h>
#include <VPR/Threads/vjThread.h>
#include <Config/vjConfigChunk.h>

#include <Kernel/vjAssert.h>

vjInput::vjInput()
 : sPort(NULL),
   instName(""),
   port_id(0),
   myThread(NULL),
   active(0),
   current(0),
   valid(1),
   progress(2),
   lock(),
   baudRate(0)
{
   //vjDEBUG(vjDBG_ALL,4)<<"*** vjInput::vjInput()\n"<< vjDEBUG_FLUSH;
   /*
   sPort = NULL;
   myThread = NULL;
   active = 0;
   */
}

vjInput::~vjInput()
{
    if (sPort != NULL)
        delete [] sPort;
}

bool vjInput::config( vjConfigChunk *c)
{
  //sPort = NULL;
  if((sPort != NULL) && (!instName.empty()))
  {
     // ASSERT: We have already been configured
     //         this prevents config from being called multiple times (once for each derived class)
     //         ie. vjDigital, vjAnalog, etc
     return true;
  }

  char* t = c->getProperty("port").cstring();
  if (t != NULL)
  {
    sPort = new char[ strlen(t) + 1 ];
    strcpy(sPort,t);
  }

  instName = (std::string)c->getProperty("name");
  baudRate = c->getProperty("baud");

  return true;
}


void vjInput::setPort(const char* serialPort)
{
if (myThread != NULL) {
     std::cerr << "Cannot change the serial Port while active\n";
     return;
  }
  strncpy(sPort,serialPort,(size_t)30);
}

char* vjInput::getPort()
{
  if (sPort == NULL) return "No port";
  return sPort;
}

void vjInput::setBaudRate(int baud)
{
  if (myThread != NULL)
     baudRate = baud;
}

/*
int vjInput::fDeviceSupport(int devAbility)
{
    return (deviceAbilities & devAbility);
}
*/

//: Reset the Index Holders
// Sets to (0,1,2) in that order
void vjInput::resetIndexes()
{
    current = 0;
    valid = 1;
    progress = 2;
    assertIndexes();
}

//: Swap the current and valid indexes (thread safe)
void vjInput::swapCurrentIndexes()
{
   vjASSERT(lock.test());       // Make sure that we have the lock when we are called
   assertIndexes();
   int tmp = current;
   current = valid;
   valid = tmp;
}

//: Swap the valid and progress indexes (thread safe)
void vjInput::swapValidIndexes()
{
   lock.acquire();
   int tmp = valid;
   valid= progress;
   progress = tmp;
   assertIndexes();
   lock.release();
}

void vjInput::assertIndexes()
{
   vjASSERT((current != progress) && (current != valid) && (progress != valid));
   vjASSERT((current >= 0) && (current <= 3));
   vjASSERT((progress >= 0) && (progress <= 3));
   vjASSERT((valid >= 0) && (valid <= 3));
}

