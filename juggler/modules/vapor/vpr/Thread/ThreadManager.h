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

#ifndef _VPR_THREAD_MANAGER_H_
#define _VPR_THREAD_MANAGER_H_
//#pragma once

#include <vpr/vprConfig.h>

#include <vpr/Thread/BaseThread.h>
#include <vpr/Thread/TSTable.h>
#include <vpr/Sync/Mutex.h>
#include <vpr/Sync/Guard.h>

#include <vpr/Util/Debug.h>
#include <vpr/Util/Singleton.h>


namespace vpr {

//----------------------------------------------------
//: Manager that maintains data about all threads.
//
// This class holds data on: <br>
// - List of all threads in system <br>
// - Thread Specific data items
//
//-----------------------------------------------------
//!PUBLIC_API:
class ThreadManager
{
public:
   //-----------------------------------------------------
   //: Called when a thread has been added to the system.
   //
   //! PRE: Manager must be locked.
   //-----------------------------------------------------
   void addThread(BaseThread* thread);

   //-----------------------------------------------------
   //: Called when a thread has been removed from the
   //+ system.
   //
   //! PRE: Manager must be locked.
   //-----------------------------------------------------
   void removeThread(BaseThread* thread);

   //-----------------------------------------------------
   //: Lock the manager so that we have complete control
   //+ to do stuff.
   // The manager should be locked whenever there is
   // about to be a change in number of threads in the
   // system.
   //-----------------------------------------------------
   void lock()
   {
      mThreadVectorMutex.acquire();
   }

   //-----------------------------------------------------
   //: Unlock the manager to allow people to use it
   //+ again.
   //-----------------------------------------------------
   void unlock()
   {
      mThreadVectorMutex.release();
   }

   //-----------------------------------------------------
   //: Dump the state of the manager to debug.
   //-----------------------------------------------------
   void debugDump();

private:
   Mutex                    mThreadVectorMutex;     //: Mutex to protect the threads vector
   std::vector<BaseThread*> mThreads;            //: List of all threads in system

public:
   //: Generate a unique key for Thread Specific data
   // This value will be used locally by each thread in the system
   long generateNewTSKey()
   {
      Guard<Mutex> guard(mTSKeyMutex);
      return mNextTSObjectKey++;
   }

private:
   Mutex     mTSKeyMutex;          //: Mutex to protect allocate of TS keys
   long      mNextTSObjectKey;  //: The next available object key


   // ----------------------- //
   // --- SINGLETON STUFF --- //
   // ----------------------- //
protected:
   //-----------------------------------------------------
   //: Constructor.  Hidden, so no instantiation is
   //+ allowed.
   //-----------------------------------------------------
   ThreadManager() : mNextTSObjectKey(0)
   {;}

vprSingletonHeader(ThreadManager);
};

}; // End of vpr namespace


#endif
