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

#include <vpr/vprConfig.h>

#include <vpr/md/NSPR/NSPRHelpers.h>
#include <vpr/Util/Assert.h>
#include <vpr/md/NSPR/IO/SelectorImplNSPR.h>

namespace vpr {


//: Add the given handle to the selector
//! PRE: handle is a valid handle
//! POST: handle is added to the handle set, and initialized to a mask of no-events
bool SelectorImplNSPR::addHandle(IOSys::Handle handle)
{
   if(getHandle(handle) != mPollDescs.end())
   {
      return false;
   }

   PRPollDesc new_desc;
   new_desc.fd = handle;
   new_desc.in_flags = 0;
   new_desc.out_flags = 0;

   mPollDescs.push_back(new_desc);

   return true;
}

//: Remove a handle from the selector
//! PRE: handle is in the selector
//! POST: handle is removed from the set of valid handles
bool SelectorImplNSPR::removeHandle(IOSys::Handle handle)
{
   std::vector<PRPollDesc>::iterator i = getHandle(handle);

   if(mPollDescs.end() == i)
   {
      return false;
   }
   else
   {
      mPollDescs.erase(i);
      return true;
   }
}

//: Set the event flags going in to the select to mask
bool SelectorImplNSPR::setIn(IOSys::Handle handle, vpr::Uint16 mask)
{
   std::vector<PRPollDesc>::iterator i = getHandle(handle);

   if(mPollDescs.end() == i)
   {
      return false;
   }

   (*i).in_flags = convertMaskVprToNspr(mask);

   return true;
}

//: Get the current in flag mask
vpr::Uint16 SelectorImplNSPR::getIn(IOSys::Handle handle)
{
   std::vector<PRPollDesc>::iterator i = getHandle(handle);

   if(mPollDescs.end() == i)
   {
      // XXX: This is VERY bad thing to do.  Need to have an error code instead
      return 0;
   }

   return convertMaskNsprToVpr((*i).in_flags);
}

//: Get the current out flag mask
vpr::Uint16 SelectorImplNSPR::getOut(IOSys::Handle handle)
{
   std::vector<PRPollDesc>::iterator i = getHandle(handle);

   if(mPollDescs.end() == i)
   {
      // XXX: This is VERY bad thing to do.  Need to have an error code instead
      return 0;
   }

   return convertMaskNsprToVpr((*i).out_flags);
}

//: Select
//! ARGS: numWithEvents - Upon completion, this holds the number of items that have events
//! ARGS: timeout - The number of msecs to select for (0 - don't wait)
Status SelectorImplNSPR::select(vpr::Uint16& numWithEvents,  const vpr::Interval timeout)
{
   vpr::Status ret_val;

   PRInt32 result;

   // Call poll - If timeout == 0, then make sure we pass 0
   result = PR_Poll(&(mPollDescs[0]), mPollDescs.size(), NSPR_getInterval(timeout) );

   if( -1 == result)
   {
      NSPR_PrintError("SelectorImplNSPR::select: Error selecting. ");
      numWithEvents = 0;
      ret_val.setCode(Status::Failure);
   }
   else if(0 == result)    // Timeout
   {
      numWithEvents = 0;
      ret_val.setCode(Status::Timeout);
   }
   //else                    // Got some

   numWithEvents = result;
   return ret_val;
}

// Get the index of the handle given
//! RETURNS: .end() - Not found, else the index to the handle in mPollDescs
std::vector<PRPollDesc>::iterator SelectorImplNSPR::getHandle(PRFileDesc const* handle)
{
   // XXX: Should probably be replaced by a map in the future for speed

   for(std::vector<PRPollDesc>::iterator i=mPollDescs.begin();
          i != mPollDescs.end();i++)
   {
      if((*i).fd == handle)
         return i;
   }

   return mPollDescs.end();
}


PRUint16 SelectorImplNSPR::convertMaskVprToNspr(vpr::Uint16 mask)
{
   PRUint16 ret_mask(0);
   if(mask & Read)
      ret_mask |= PR_POLL_READ;
   if(mask & Write)
      ret_mask |= PR_POLL_WRITE;
   if(mask & Except)
      ret_mask |= PR_POLL_EXCEPT;
   if(mask & Error)
      ret_mask |= PR_POLL_ERR;
   if(mask & Invalid)
      ret_mask |= PR_POLL_NVAL;

   return ret_mask;
}

vpr::Uint16 SelectorImplNSPR::convertMaskNsprToVpr(PRUint16 mask)
{
   vpr::Uint16 ret_mask(0);
   if(mask & PR_POLL_READ)
      ret_mask |= Read;
   if(mask & PR_POLL_WRITE)
      ret_mask |= Write;
   if(mask & PR_POLL_EXCEPT)
      ret_mask |= Except;
   if(mask & PR_POLL_ERR)
      ret_mask |= Error;
   if(mask & PR_POLL_NVAL)
      ret_mask |= Invalid;

   return ret_mask;
}

}  // namespace vpr
