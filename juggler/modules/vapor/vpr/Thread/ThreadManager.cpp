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

#include <Utils/Assert.h>
#include <Threads/ThreadManager.h>


namespace vpr {

//ThreadManager* ThreadManager::_instance = NULL;
vprSingletonImp(ThreadManager);


//-----------------------------------------------------
//: Called when a thread has been added to the system.
//
//! PRE: Manager must be locked.
//-----------------------------------------------------
void ThreadManager::addThread(BaseThread* thread)
{
   vprASSERT(mThreadVectorMutex.test()==1); // Assert manager locked
   vprASSERT(thread->getTID() >= 0);

   // Insert thread into local table
   while ((int)mThreads.size() <= thread->getTID())
      mThreads.push_back(NULL);
   mThreads[thread->getTID()] = thread;

   debugDump();               // Dump current state
}

//-----------------------------------------------------
//: Called when a thread has been removed from the
//+ system.
//
//! PRE: Manager must be locked.
//-----------------------------------------------------
void ThreadManager::removeThread(BaseThread* thread)
{
   vprASSERT(mThreadVectorMutex.test()==1); // Assert manager locked
   vprASSERT((thread->getTID() >= 0) && (thread->getTID() < (int)mThreads.size()));
   mThreads[(unsigned int)thread->getTID()] = NULL;
}


//: Dump the state of the manager to debug
void ThreadManager::debugDump()
{
   vprDEBUG(vprDBG_ALL, vprDBG_VERB_LVL)
      << "------- Thread Manager DUMP -------\n" << vprDEBUG_FLUSH;
   vprDEBUG_BEGIN(vprDBG_ALL, vprDBG_STATE_LVL) << "--- Thread List ----\n";
   for (unsigned int i=0;i<mThreads.size();i++)
   {
      if (mThreads[i] != NULL)
         vprDEBUGnl(vprDBG_ALL, vprDBG_STATE_LVL) << i << ": ["
                                                << (void*)mThreads[i] << "] "
                                                << mThreads[i] << std::endl;
      else
         vprDEBUGnl(vprDBG_ALL, vprDBG_STATE_LVL) << i << ": ["
                                                << (void*)mThreads[i]
                                                << "] No thread\n";
   }

   vprDEBUG_ENDnl(vprDBG_ALL, vprDBG_STATE_LVL) << "---------------------\n"
                                              << vprDEBUG_FLUSH;
}

}; // End of vpr namespace
