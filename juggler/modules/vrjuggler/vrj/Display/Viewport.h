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


#ifndef _VJ_VIEWPORT_H_
#define _VJ_VIEWPORT_H_

#include <vjConfig.h>

#include <Math/vjVec3.h>
#include <Input/vjPosition/vjPosition.h>
#include <Kernel/vjUser.h>
#include <Environment/vjEnvironmentManager.h>
#include <Performance/vjPerfDataBuffer.h>

// Config stuff
class vjConfigChunk;


//---------------------------------------------------------------------
//: Base class for window viewports
//
// Base class for all viewport data
//
// Stores projection data, viewport info, and relevant user
//
// @author Allen Bierbaum
//  Date: 3-5-2001
//-----------------------------------------------------------------------
class vjViewport
{
public:
   vjViewport() : mUser(NULL), mViewportChunk(NULL)
   {
      mXorigin = mYorigin = mXsize = mYsize = -1.0f;
      mType = vjViewport::UNDEFINED;
      mActive = false;
   }

   virtual ~vjViewport()
   {;}


   enum Type { UNDEFINED, SURFACE, SIM};                  // What type of viewport is it
   enum View { NONE=0, LEFT_EYE=1, RIGHT_EYE=2, STEREO=3 };      // For referring to which eye(s) to draw

public:
      //: Takes a viewport chunk and configures the display based one it.
      //! PRE: chunk is a valid chunk
      //! POST: viewport is configured
      //+       If there is an error is the specified config, we output error
      //+       and "fix" the error.
      //! NOTE: All derived viewport classes MUST call this function
      //+       after doing local configuration.
   virtual void config(vjConfigChunk* chunk);

   //: Updates the projection data for this display
   // Uses the data for the head position for this window
   virtual void updateProjections() = 0;

public:
   vjViewport::Type getType()
   { return mType;}

   bool isSimulator()
   { return (mType == SIM); }

   bool isSurface()
   { return (mType == SURFACE); }

   bool isActive()
   { return mActive; }

    void setName(std::string name)
   { mName = name; }

   //: Get the name of the display
   std::string getName()
   { return mName;}

   //!NOTE: If we are in simulator, we can not be in stereo
   bool inStereo()
   { return (mView == STEREO); }

   // Which view are we supposed to render
   vjViewport::View getView()
   { return mView; }

   void setOriginAndSize(float xo, float yo, float xs, float ys)
   { mXorigin = xo; mYorigin = yo; mXsize = xs; mYsize = ys;}
   void getOriginAndSize(float& xo, float& yo, float& xs, float& ys)
   {
      xo = mXorigin; yo = mYorigin; xs = mXsize; ys = mYsize;
   }

   //: Get the config chunk that configured this display
   vjConfigChunk* getConfigChunk()
   { return mViewportChunk; }

   //: Get the user associated with this display
   vjUser*  getUser()
   { return mUser;}

   virtual std::ostream& outStream(std::ostream& out);
   friend std::ostream& operator<<(std::ostream& out, vjViewport& viewport);

protected:
   std::string       mName;               //: The name of the viewport being displayed
   vjUser*           mUser;               //: The user being rendered by this window
   vjViewport::Type  mType;               //: The type of display
   vjViewport::View  mView;               //: Which buffer(s) to display (left, right, stereo)
   bool              mActive;             //: Is this viewport active

   vjConfigChunk* mViewportChunk;        //: The chunk data for this display
   float          mXorigin, mYorigin, mXsize, mYsize;    // Location and size of viewport
                                                         // ASSERT: all values are >= 0.0 and <= 1.0
};


#endif
