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

#include <gadget/gadgetConfig.h>

#include <vpr/Thread/Thread.h>
#include <vpr/Util/Assert.h>

#include <gadget/Type/Input.h>
#include <jccl/Config/ConfigChunk.h>


namespace gadget
{

Input::Input()
 : mPort(NULL),
   mInstName(""),
   mPortId(0),
   mThread(NULL),
   mActive(0),
   current(0),
   valid(1),
   progress(2),
   lock(),
   mBaudRate(0)
{
}

Input::~Input()
{
    if (mPort != NULL)
        delete [] mPort;
}

bool Input::config( jccl::ConfigChunkPtr c)
{
  //mPort = NULL;
  if((mPort != NULL) && (!mInstName.empty()))
  {
     // ASSERT: We have already been configured
     //         this prevents config from being called multiple times (once for each derived class)
     //         ie. Digital, Analog, etc
     return true;
  }

  char* t = c->getProperty("port").cstring();
  if (t != NULL)
  {
    mPort = new char[ strlen(t) + 1 ];
    strcpy(mPort,t);
  }

  mInstName = (std::string)c->getProperty("name");
  mBaudRate = c->getProperty("baud");

  return true;
}


void Input::setPort(const char* serialPort)
{
if (mThread != NULL) {
     std::cerr << "Cannot change the serial Port while active\n";
     return;
  }
  strncpy(mPort,serialPort,(size_t)30);
}

char* Input::getPort()
{
  if (mPort == NULL) return "No port";
  return mPort;
}

void Input::setBaudRate(int baud)
{
  if (mThread != NULL)
     mBaudRate = baud;
}


};
