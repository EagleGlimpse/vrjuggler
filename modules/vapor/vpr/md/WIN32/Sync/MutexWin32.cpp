/****************** <VPR heading BEGIN do not edit this line> *****************
 *
 * VR Juggler Portable Runtime
 *
 * Original Authors:
 *   Allen Bierbaum, Patrick Hartling, Kevin Meinert, Carolina Cruz-Neira
 *
 ****************** <VPR heading END do not edit this line> ******************/

/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2006 by Iowa State University
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
 *************** <auto-copyright.pl END do not edit this line> ***************/

#include <vpr/vprConfig.h>

#include <sstream>

#include <vpr/Util/ResourceException.h>
#include <vpr/Util/Error.h>
#include <vpr/md/WIN32/Sync/MutexWin32.h>


namespace vpr
{

MutexWin32::MutexWin32()
   : mMutex(NULL)
   , mLocked(false)     // We don't have it locked yet
{        
   // ----- Allocate the mutex ----- //
   // NULL - No security
   // FALSE - We don't want to acquire it
   // NULL - Unamed version
   mMutex = CreateMutex(NULL, FALSE, NULL);

   if ( mMutex == NULL || mMutex == INVALID_HANDLE_VALUE )
   {
      std::ostringstream msg_stream;
      msg_stream << "Mutex allocation failed: "
                 << vpr::Error::getCurrentErrorMsg();
      throw vpr::ResourceException("Mutex allocation failed", VPR_LOCATION);
   }
}

} // End of vpr namespace