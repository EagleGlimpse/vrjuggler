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


#ifndef _VJ_SURFACE_DISPLAY_H_
#define _VJ_SURFACE_DISPLAY_H_

#include <Utils/vjDebug.h>
#include <Kernel/vjViewport.h>
#include <Kernel/vjProjection.h>
#include <Math/vjVec3.h>
class vjConfigChunk;

//: Defines a display surface an associated projections
//
//
class vjSurfaceViewport : public vjViewport
{
public:
   vjSurfaceViewport() :  mTracked(false), mLeftProj(NULL), mRightProj(NULL)
   {;}

   virtual ~vjSurfaceViewport() {}

public:
   //: Takes a display chunk and configures the display based one it.
   //! PRE: chunk is a valid chunk
   //! POST: display is configured
   //+       If there is an error is the specified config, we output error
   //+       and "fix" the error.
   //! NOTE: All derived display classes MUST call this function
   //+       after doing local configuration.
   virtual void config(vjConfigChunk *chunk);

   virtual void updateProjections();

   void getCorners(vjVec3& ll, vjVec3& lr, vjVec3& ur, vjVec3& ul)
   {
      ll = mLLCorner; lr = mLRCorner; ur = mURCorner; ul = mULCorner;
   }

   vjProjection* getLeftProj()
   { return mLeftProj; }

   vjProjection* getRightProj()
   { return mRightProj; }

   virtual std::ostream& outStream(std::ostream& out)
   {
      vjViewport::outStream(out);

      out << "LL: " << mLLCorner << ", LR: " << mLRCorner
          << ", UR: " << mURCorner << ", UL:" << mULCorner << std::endl;
      out << "surfRot: \n" << mSurfaceRotation << std::endl;
      out << "leftProj: " << *mLeftProj;
      out << "  rightProj: " << *mRightProj << std::endl;
      return out;
   }

protected:
   void calculateSurfaceRotation();

   void calculateCornersInBaseFrame();

   //: Check the pts to make sure they form a legal surface
   void assertPtsLegal()
   {
      vjVec3 norm1, norm2;
      vjVec3 bot_side = mLRCorner-mLLCorner;
      vjVec3 diag = mULCorner-mLRCorner;
      vjVec3 right_side = mURCorner-mLRCorner;
      norm1 = bot_side.cross(diag);
      norm2 = bot_side.cross(right_side);
      norm1.normalize(); norm2.normalize();
      if(norm1 != norm2)
         vjDEBUG(vjDBG_ERROR,0) << "ERROR: Invalid surface corners.\n" << vjDEBUG_FLUSH;
   }


protected:
   vjVec3   mLLCorner, mLRCorner, mURCorner, mULCorner;  //: The corners in 3Space (for config)
   vjMatrix mSurfaceRotation;                            //: surfMbase - rotation to base coordinate frame of the surface view plane

   // Deal with tracked surfaces (ie. HMD, movable walls, desks, etc)
   bool           mTracked;            // Is this surface tracked
   std::string    mTrackerProxyName;   // If tracked, what is the name of the tracker

   /// Defines the projection for this window. Ex. RIGHT, LEFT, FRONT
   vjProjection*   mLeftProj;              //: Left eye projection
   vjProjection*   mRightProj;             //: Right eye projection

private:
         // These values are used to compute the coordinates of the view plane
         // in the transformed coord system of mSurfaceRotation
   vjVec3   mxLLCorner, mxLRCorner, mxURCorner, mxULCorner;    //: The corners transformed onto an x,y plane
};


#endif
