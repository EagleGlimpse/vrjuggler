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

#include <vrj/vrjConfig.h>
//#include <sys/types.h>

#include <vrj/Display/WallProjection.h>
#include <vrj/Kernel/Kernel.h>

#include <gmtl/Vec.h>
#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/Generate.h>
#include <gmtl/Xforms.h>


//#include <vrj/Math/Coord.h>
#include <vrj/Util/Debug.h>

#include <jccl/Config/ConfigChunk.h>

namespace vrj
{


// Just call the base class constructor
void WallProjection::config(jccl::ConfigChunkPtr chunk)
{
   vprASSERT( ((std::string)chunk->getType() == std::string("surfaceDisplay")) ||
             ((std::string)chunk->getType() == std::string("surfaceViewport")) );

   Projection::config(chunk);        // Call base class config first
}

//: Recalculate the projection matrix
// Uses a method that needs to know the distance in the screen plane
// from the origin (determined by the normal to the plane throught the
// origin) to the edges of the screen.
// This method can be used for any rectangular planar screen.
// By adjusting the wall rotation matrix, this method can be used for
// the general case of a rectangular screen in 3-space
//
//! PRE: WallRotation matrix must be set correctly
//! PRE: mOrigin*'s must all be set correctly
void WallProjection::calcViewMatrix(gmtl::Matrix44f& eyePos)
{
   calcViewFrustum(eyePos);

   //Coord eye_coord(eyePos);
   gmtl::Vec3f   eye_pos( gmtl::makeTrans<gmtl::Vec3f>(eyePos) );             // Non-xformed pos
   //eye_pos = eye_coord.pos;

   // Need to post translate to get the view matrix at the position of the eye
   mViewMat = mWallRotationMatrix *
              gmtl::makeTrans<gmtl::Matrix44f>( gmtl::Vec3f(-eye_pos[gmtl::Xelt], -eye_pos[gmtl::Yelt], -eye_pos[gmtl::Zelt]));
}


//: Recalculate the view frustum
// Uses a method that needs to know the distance in the screen plane
// from the origin (determined by the normal to the plane throught the
// origin) to the edges of the screen.
// This method can be used for any rectangular planar screen.
// By adjusting the wall rotation matrix, this method can be used for
// the general case of a rectangular screen in 3-space
//
//! PRE: WallRotation matrix must be set correctly
//! PRE: mOrigin*'s must all be set correctly
void WallProjection::calcViewFrustum(gmtl::Matrix44f& eyePos)
{
   float near_dist, far_dist;
   near_dist = mNearDist;
   far_dist = mFarDist;

   // Distance measurements from eye to screen/edges
   // Distance to edges is from the point on the screen plane
   // where a normal line would go through the origin.
   float eye_to_screen, eye_to_right, eye_to_left, eye_to_top, eye_to_bottom;

   // Distances in near plane, in near plane from origin.  (Similar to above)
   float n_eye_to_right, n_eye_to_left, n_eye_to_top, n_eye_to_bottom;


   //Coord eye_coord(eyePos);
   gmtl::Point3f   eye_pos;             // Non-xformed pos
   eye_pos = gmtl::makeTrans<gmtl::Point3f>(eyePos);
   gmtl::Point3f   eye_xformed;         // Xformed position of eyes

   vprDEBUG(vrjDBG_DISP_MGR,7)
      << "vjWallProjection::calcWallProjection:  Wall Proj:\n" << *this
      << std::endl << vprDEBUG_FLUSH;

   /*
   vprDEBUG(vrjDBG_DISP_MGR,7)
      << "vjWallProjection::calcWallProjection:    Base eye:" << eye_pos
      << std::endl << vprDEBUG_FLUSH;
      */

   // Convert eye coords into the wall's coord system
   eye_xformed = mWallRotationMatrix * eye_pos;

   /*
   vprDEBUG(vrjDBG_DISP_MGR,7)
      << "vjWallProjection::calcWallProjection:    Xformed eye:" << eye_xformed
      << std::endl << vprDEBUG_FLUSH;
      */

   // Compute dist from eye to screen/edges
   eye_to_screen = mOriginToScreen + eye_xformed[gmtl::Zelt];
   eye_to_right = mOriginToRight - eye_xformed[gmtl::Xelt];
   eye_to_left = mOriginToLeft + eye_xformed[gmtl::Xelt];
   eye_to_top = mOriginToTop - eye_xformed[gmtl::Yelt];
   eye_to_bottom = mOriginToBottom + eye_xformed[gmtl::Yelt];

   // Find dists on near plane using similar triangles
   float near_distFront = near_dist/eye_to_screen;      // constant factor
   n_eye_to_left = eye_to_left*near_distFront;
   n_eye_to_right = eye_to_right*near_distFront;
   n_eye_to_top = eye_to_top*near_distFront;
   n_eye_to_bottom = eye_to_bottom*near_distFront;

   // Set frustum and calulcate the matrix
   mFrustum.set(-n_eye_to_left, n_eye_to_right,
                -n_eye_to_bottom, n_eye_to_top,
                near_dist, far_dist);

   mFocusPlaneDist = eye_to_screen;    // Needed for drawing

   vprDEBUG(vrjDBG_DISP_MGR,7)
      << "vjWallProjection::calcWallProjection: \n\tFrustum: " << mFrustum
      << std::endl << vprDEBUG_FLUSH;
   /*
   vprDEBUG(vrjDBG_DISP_MGR,7)
      << "vjWallProjection::calcWallProjection: B4 Trans:\n"
      << mWallRotationMatrix << std::endl << vprDEBUG_FLUSH;
      */
}


std::ostream& WallProjection::outStream(std::ostream& out)
{
   out << "vjWallProjection: ";

   return Projection::outStream(out);
}


};
