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


#ifndef _VJ_QUAT_H_
#define _VJ_QUAT_H_

#include <math.h>

#include <Math/vjVec4.h>
#include <Math/vjVec3.h>
#include <Math/vjMatrix.h>
#include <VPR/vjSystem.h>

#define VJ_QUAT_EPSILON     0.00001f

//: Quaternion
//  A quaternion is a data structure used to store arbitrary rotation
//  in 3space.  It is useful in robotics and animation, and is much faster
//  to use than a rotation matrix when doing product or quotient operations
//  (i.e. multiply or divide).
//
// References (alphabetical):
//    Advanced Animation and Rendering Techniques: pp363-365           <br>
//    Animating Rotation with Quaternion Curves,  Ken Shoemake         <br>
//       SIGGRAPH Proceedings Vol 19, Number 3, 1985                   <br>
//    Game Developer Magazine: Feb 98, pg.34-42                        <br>
//    Motivation for the use of Quaternions to perform transformations <br>
//       http://www.rust.net/~kgeoinfo/quat1.htm                       <br>
//    On quaternions; or on a new system of imaginaries in algebra,    <br>
//       Sir William Rowan Hamilton, Philosophical Magazine, xxv,      <br>
//       pp. 10-13 (July 1844)                                         <br>
//    Quaternion Calculus for Animation,  Ken Shoemake SIGGRAPH course notes 1989  <br>
//
//!PUBLIC_API:
class vjQuat
{
public:
   //: default constructor
   //!POST: initialize self to identity
   vjQuat();
   
   //: Construct self from another vjQuat 
   vjQuat( const vjQuat& quat );

   //: Construct Quat from a pure quaternion 
   // NOTE: a pure quaternion is one with scalar component set to 0
	vjQuat( const vjVec3& pure_quat );
   
   //: Construct Quat from 4 floats
   vjQuat( const float w,
           const float x,
           const float y,
           const float z );
   
   //: set self to 0 rotation
   void identity();
   
   //: copy the quaternion data from scalar: [w]  vec: [x y z]
   //!POST: self = {w,x,y,z}
   void copy( float w, float x, float y, float z );
   
   //: copy the quaternion to self
   //!POST: self = quat
   void copy( const vjQuat& quat );
   
   //: Copy a pure quaternion to self
   // NOTE: a pure quaternion is one with scalar component set to 0
	//!POST: self = pure_quat
   void copy( const vjVec3& pure_quat );
   
   //: return the scalar component
   const float& scalar() const;

   //: return the vector component
   vjVec3 vector() const;

   //: returns the quaternion's norm (dot product)
   // defined as sum of squares of all quat components
   float norm() const;
     
   //: returns the quaternion's magnitude (also called absolute)
   // defined as the norm of all four quaternion components
   float length() const;
   
   //: set self to the complex conjugate of self.
	void conj();
   
   //: set self to the complex conjugate of quat.
	void conj( const vjQuat& quat );
  
   //: normalize without the square root.  faster but not precise...
   //!POST: self = self / norm()
   //       normalizeFast() makes quat.norm() == 1
   void normalizeFast();
   
   //: set self to the unit quaternion of quat. fast, imprecise
	void normalizeFast( const vjQuat& quat );
   
   //: set self to the normalized quaternion of self.
   //!POST: self = normalize(self), where normalize makes quat.length() == 1
   void normalize();
   
   //: set self to the normalized quaternion of quat.
	void normalize( const vjQuat& quat );
  
   //: set self to the multiplicative inverse of self
   //!POST: self becomes the multiplicative inverse of self
   void invert();
   
	//: set self to the multiplicative inverse of quat
	//!POST: self becomes the multiplicative inverse of quat
   void invert( const vjQuat& quat );

public:
   //: product of two quaternions
   //!POST: this' = q1 * q2
   void mult( const vjQuat& q1, const vjQuat& q2 );

   //: scalar multiplication
   //!POST: this' = q1 * s
   void mult( const vjQuat& q1, const float& s );

   //: quotient of two quaternions
   //!POST: this' = q1 / q2
   void div( const vjQuat& q1, const vjQuat& q2 );

   // do an "add" of all quat components
   // TODO: add description for what this means geometrically
   void add( const vjQuat& quat1, const vjQuat& quat2 );
   
   // set self to quat1 - quat2
   // TODO: add description for what this means geometrically
   void sub( const vjQuat& quat1, const vjQuat& quat2 );
   
   //: complex exponentiation 
   //!POST: sets self to the exponentiation of quat
   // NOTE: safe to pass self as argument
   void exp( const vjQuat& quat );
   
   //: complex logarithm
   //!POST: sets self to the log of quat
   void log( const vjQuat& quat );
   
   //: check if self is equal to quat within some tolerance 'tol'
   //!PRE:  give one quaternion to test for equality, and a tolerance
   //!PRE:  tolerance defaults to VJ_QUAT_EPSILON if none given.
   //!POST: returns true if the two quats are almost equal
   bool almostEqual( const vjQuat& quat, float tol = VJ_QUAT_EPSILON ) const;
   
   //: Set self to the rotation component contained 
   //:  within the given rotation matrix
   void makeRot( const vjMatrix& mat );
   
   //: make a quat from a twist (radians) about a vector (normalized)
   void makeRot( const float& rad, const float& x, const float& y, const float& z );
 
   //: get the quat's twist (radians) and vector
   void getRot( float& rad, float& xx, float& yy, float& zz ) const;
   
   //: Spherical quadratic interpolation
   // WARNING: not implemented (do not use)
   // Spherical quadratic interpolation is provided by squad and its
   // helper function, meanTangent.
   void squad( float t, const vjQuat& q1, const vjQuat& q2, const vjQuat& a, const vjQuat& b );

   // WARNING: not implemented (do not use)
   void meanTangent( const vjQuat &q1, const vjQuat &q2, const vjQuat &q3 );
   
   //: spherical linear interpolation
   void slerp( float t, const vjQuat& q1, const vjQuat& q2 );
   
   
public:
   //: compose two rotations (multiplication operator) 
   //!POST: temporary return value = self * quat 
   vjQuat operator*( const vjQuat& quat ) const;
   
   //: multiply by a pure quaternion (only vector, scalar component == 0)
   //  returns a pure quaternion as temporary
   // NOTE: a pure quaternion is one with scalar component set to 0
	//!PRE: give a pure quaternion for "pure_quat"
   //!POST: pure quaternion = self * pure_quat 
   vjVec3 operator*( const vjVec3& pure_quat ) const;
   
   // mult by scalar 
   // NOTE: less efficient, use mult() instead
	vjQuat operator*( float s ) const;
   
   // multiply by inverse quat.
   // NOTE: less efficient, use div() instead
	vjQuat operator/( const vjQuat& quat ) const;
   
   // mult by inverse scalar.
   //!POST: returns a temporary == self scaled by s
   vjQuat operator/( float s ) const;

   // do an "add" of all quat components
   // TODO: add description for what this means geometrically
   vjQuat operator+( const vjQuat& quat ) const;
   
	// do a "subtract" of all quat components
   // TODO: add description for what this means geometrically
   vjQuat operator-( const vjQuat& quat ) const;
   
public:
   // Assignment operator *=
   vjQuat& operator*=( const vjQuat& quat );

   // Assignment operator /=
   vjQuat& operator/=( const vjQuat& quat );
   
   // does self == quat?
   // return true if self == quat
   // return false if self != quat
   bool operator==( const vjQuat& quat );
   
	// does self != quat?
   // return true if self != quat
   // return false if self == quat
   bool operator!=( const vjQuat& quat );
   
   //: set self to quat
	vjQuat& operator=( const vjQuat& quat );
   
   //: set self to 'pure' quaternion pure_quat
   // NOTE: a pure quaternion is one with scalar component set to 0
	vjQuat& operator=( const vjVec3& pure_quat );
   
   //: output method
   std::ostream& outStreamRaw( std::ostream& out ) const;
   
   //: output method
   std::ostream& outStreamReadable( std::ostream& out ) const;
   
   //: output operator
   friend std::ostream& operator<<( std::ostream& out, const vjQuat& q );
   
   //: quaternion data access for external function use (like quat to matrix)
   // non-const version
   // use VJ_W, VJ_X, VJ_Y, or VJ_Z to access each component
   //
   float& operator[]( int x );
   
   //: quaternion data access for external function use (like quat to matrix)
   // const version
   // use VJ_W, VJ_X, VJ_Y, or VJ_Z to access each component
   //
   const float& operator[]( int x ) const;
   
private:
   vjVec4 vec;
};


////////////////////////////////////////////////////////////////////////////
// Implementation of inline funcs...
////////////////////////////////////////////////////////////////////////////


//: return the scalar component
inline const float& vjQuat::scalar() const { return vec[VJ_W]; }

//: return the vector component
inline vjVec3 vjQuat::vector() const { return vjVec3( vec[VJ_X], vec[VJ_Y], vec[VJ_Z] ); }

//: quaternion data access for external function use (like quat to matrix)
// non-const version
// use VJ_W, VJ_X, VJ_Y, or VJ_Z to access each component
//
inline float& vjQuat::operator[]( int x )
{ 
   return vec[x]; 
}

//: quaternion data access for external function use (like quat to matrix)
// const version
// use VJ_W, VJ_X, VJ_Y, or VJ_Z to access each component
//
inline const float& vjQuat::operator[]( int x ) const 
{ 
   return vec[x];
}

//: set self to 0 rotation
inline void vjQuat::identity()
{
   vec[VJ_W] = 0.0f;
   vec[VJ_X] = 1.0f;
   vec[VJ_Y] = 0.0f;
   vec[VJ_Z] = 0.0f;
}



//: copy the quaternion data from scalar: [w]  vec: [x y z]
//!POST: self = {w,x,y,z}
inline void vjQuat::copy( float w, float x, float y, float z )
{
   vec[VJ_W] = w;
   vec[VJ_X] = x;
   vec[VJ_Y] = y;
   vec[VJ_Z] = z;
}   

//: copy the quaternion to self
//!POST: self = quat
inline void vjQuat::copy( const vjQuat& quat )
{
   vec = quat.vec;
}   

//: Copy a pure quaternion to self
// NOTE: a pure quaternion is one with scalar component set to 0
//!POST: self = pure_quat
inline void vjQuat::copy( const vjVec3& pure_quat )
{
   vec[VJ_X] = 0.0f;
   vec[VJ_Y] = pure_quat[0];
   vec[VJ_Z] = pure_quat[1];
   vec[VJ_W] = pure_quat[2];
}



//: default constructor
//!POST: initialize self to identity
inline vjQuat::vjQuat() : vec()
{
   this->identity();
}

//: Construct self from another vjQuat 
inline vjQuat::vjQuat( const vjQuat& quat ) : vec()
{
   this->copy( quat );
}

//: Construct Quat from a pure quaternion 
// NOTE: a pure quaternion is one with scalar component set to 0
inline vjQuat::vjQuat( const vjVec3& pure_quat ) : vec()
{
   this->copy( pure_quat );
}

//: Construct Quat from 4 floats
inline vjQuat::vjQuat( const float w,
        const float x,
        const float y,
        const float z ) : vec()
{
   this->copy( w, x, y, z );
}

//: returns the quaternion's norm (dot product)
// defined as sum of squares of components
inline float vjQuat::norm() const
{
   return ((vec[0]*vec[0])+(vec[1]*vec[1])+(vec[2]*vec[2])+(vec[3]*vec[3]));
}

//: returns the quaternion's magnitude (also called absolute)
// defined as the norm of all four quaternion components
inline float vjQuat::length() const { return vjSystem::sqrt( this->norm() ); }

//: set self to the complex conjugate of self.
inline void vjQuat::conj()
{ 
   vec[VJ_X] = -vec[VJ_X];
   vec[VJ_Y] = -vec[VJ_Y];
   vec[VJ_Z] = -vec[VJ_Z];
}

//: set self to the complex conjugate of quat.
inline void vjQuat::conj( const vjQuat& quat )
{ 
   this->copy( quat );
   this->conj();
}

//: normalize without the square root.  faster but not precise...
//!POST: self = self / norm()
//       normalizeFast() makes quat.norm() == 1
inline void vjQuat::normalizeFast()
{ 
   float n = 1.0f / this->norm();
   if (n >= VJ_QUAT_EPSILON)
   {
      vec[VJ_W] *= n; 
      vec[VJ_X] *= n; 
      vec[VJ_Y] *= n; 
      vec[VJ_Z] *= n;
   }
   else
   {
      // set to something safe, since the quat's vector has no length
      // no length implies an invalid quaternion
      // TODO: should we at least copy over the scalar component?
      //       one reason not to is that the quat is invalid, because of its vec
      this->copy( 0.0f, 1.0f, 0.0f, 0.0f );
   }
}

//: set self to the unit quaternion of quat. fast, imprecise
inline void vjQuat::normalizeFast( const vjQuat& quat )
{ 
   this->copy( quat );
   this->normalizeFast();
}

//: set self to the normalized quaternion of self.
//!POST: self = normalize(self), where normalize makes quat.length() == 1
inline void vjQuat::normalize()
{ 
   float n = 1.0f / this->length();
   if (n >= VJ_QUAT_EPSILON)
   {
      vec[VJ_W] *= n; 
      vec[VJ_X] *= n; 
      vec[VJ_Y] *= n; 
      vec[VJ_Z] *= n;
   }
   else
   {
      // set to something safe, since the quat's vector has no length
      // no length implies an invalid quaternion
      // TODO: should we at least copy over the scalar component?
      //       one reason not to is that the quat is invalid, because of its vec
      this->copy( 0.0f, 1.0f, 0.0f, 0.0f );
   }
}

//: set self to the normalized quaternion of quat.
inline void vjQuat::normalize( const vjQuat& quat )
{ 
   this->copy( quat );
   this->normalize();
}

//: set self to the multiplicative inverse of self
//!POST: self becomes the multiplicative inverse of self
inline void vjQuat::invert()
{
   this->conj(); 
   this->normalizeFast();
}

//: set self to the multiplicative inverse of quat
//!POST: self becomes the multiplicative inverse of quat
inline void vjQuat::invert( const vjQuat& quat )
{ 
   this->copy( quat );
   this->invert();
}

//: scalar multiplication
//!POST: this' = q1 * s
inline void vjQuat::mult( const vjQuat& q1, const float& s )
{
   vec[VJ_W] = q1.vec[VJ_W] * s;
   vec[VJ_X] = q1.vec[VJ_X] * s;
   vec[VJ_Y] = q1.vec[VJ_Y] * s;
   vec[VJ_Z] = q1.vec[VJ_Z] * s;
}

//: quotient of two quaternions
//!POST: this' = q1 / q2
inline void vjQuat::div( const vjQuat& q1, const vjQuat& q2 )
{
   vjQuat q2_inv, r, s;

   // invert the vector component
   q2_inv.conj( q2 );

   r.mult( q1, q2_inv );
   s.mult( q2_inv, q2_inv );

   vec[VJ_X] = r.vec[VJ_X] / s.vec[VJ_W];
   vec[VJ_Y] = r.vec[VJ_Y] / s.vec[VJ_W];
   vec[VJ_Z] = r.vec[VJ_Z] / s.vec[VJ_W];
   vec[VJ_W] = r.vec[VJ_W] / s.vec[VJ_W];
}

//: check if two quaternions are equal within some tolerance 'tol'
// WARNING: not implemented (do not use)
inline bool vjQuat::almostEqual( const vjQuat& quat, float tol ) const
{
   if (VJ_ISEQUAL( vec[0], quat.vec[0], tol ) && 
       VJ_ISEQUAL( vec[1], quat.vec[1], tol ) &&
       VJ_ISEQUAL( vec[2], quat.vec[2], tol ) &&
       VJ_ISEQUAL( vec[3], quat.vec[3], tol ) )
   {
      return true;
   }
   else
   {
      return false;
   }
}

// WARNING: not implemented (do not use)
inline void vjQuat::meanTangent( const vjQuat &q1, const vjQuat &q2, const vjQuat &q3 )
{
   assert( false && "not implemented" ); 
}   

//: compose two rotations (multiplication operator) 
//!POST: temporary return value = self * quat 
inline vjQuat vjQuat::operator*( const vjQuat& quat ) const 
{
   vjQuat dst; 
   dst.mult( *this, quat ); 
   return dst;
}

//: multiply by a pure quaternion (only vector, scalar component == 0)
//  returns a pure quaternion as temporary
// NOTE: a pure quaternion is one with scalar component set to 0
//!PRE: give a pure quaternion for "pure_quat"
//!POST: pure quaternion = self * pure_quat 
inline vjVec3 vjQuat::operator*( const vjVec3& pure_quat ) const
{
   // convert the vec3 to a quat
   vjQuat pure( pure_quat );

   // get the inverse of self
   vjQuat self_inv;
   self_inv.invert( *this );

   // do the multiplication, and return
   vjQuat result = self_inv * pure * (*this);
	return vjVec3( result.vec[VJ_X], result.vec[VJ_Y], result.vec[VJ_Z] );
}

// mult by scalar 
// NOTE: less efficient, use mult() instead
inline vjQuat vjQuat::operator*( float s ) const
{
   vjQuat result;
   result.mult( *this, s );
   return result;
}

// multiply by inverse quat.
// NOTE: less efficient, use div() instead
inline vjQuat vjQuat::operator/( const vjQuat& quat ) const 
{
   vjQuat dst; 
   dst.div( *this, quat );
   return dst;
}

// mult by inverse scalar.
//!POST: returns a temporary == self scaled by s
inline vjQuat vjQuat::operator/( float s ) const
{
   s = 1.0f / s;
   vjQuat result;
   result.mult( *this, s );
   return result;
}   

// do an "add" of all quat components
// TODO: add description for what this means geometrically
inline void vjQuat::add( const vjQuat& quat1, const vjQuat& quat2 )
{
   vec[VJ_W] = quat1.vec[VJ_W] + quat2.vec[VJ_W];
   vec[VJ_X] = quat1.vec[VJ_X] + quat2.vec[VJ_X];
   vec[VJ_Y] = quat1.vec[VJ_Y] + quat2.vec[VJ_Y];
   vec[VJ_Z] = quat1.vec[VJ_Z] + quat2.vec[VJ_Z];
}

// set self to quat1 - quat2
// TODO: add description for what this means geometrically
inline void vjQuat::sub( const vjQuat& quat1, const vjQuat& quat2 )
{
   vec[VJ_W] = quat1.vec[VJ_W] - quat2.vec[VJ_W];
   vec[VJ_X] = quat1.vec[VJ_X] - quat2.vec[VJ_X];
   vec[VJ_Y] = quat1.vec[VJ_Y] - quat2.vec[VJ_Y];
   vec[VJ_Z] = quat1.vec[VJ_Z] - quat2.vec[VJ_Z];
}

// do an "add" of all quat components
// TODO: add description for what this means geometrically
inline vjQuat vjQuat::operator+( const vjQuat& quat ) const
{
   vjQuat result;
   result.add( *this, quat );
   return result;
}

// do a "subtract" of all quat components
// TODO: add description for what this means geometrically
inline vjQuat vjQuat::operator-( const vjQuat& quat ) const
{
   vjQuat result;
   result.sub( *this, quat );
   return result;
}   

// Assignment operator *=
inline vjQuat& vjQuat::operator*=( const vjQuat& quat )
{
   this->mult( *this, quat );
   return *this;
}
// Assignment operator /=
inline vjQuat& vjQuat::operator/=( const vjQuat& quat ) 
{
   this->div( *this, quat );
   return *this;
}

// does self == quat?
// return true if self == quat
// return false if self != quat
inline bool vjQuat::operator==( const vjQuat& quat )
{
   if (quat.scalar() == this->scalar() &&
       quat.vector() == this->vector())
   {
      return true;
   }      
   else
   {
      return false;
   }       
} 

// does self != quat?
// return true if self != quat
// return false if self == quat
inline bool vjQuat::operator!=( const vjQuat& quat )
{
   if (quat.scalar() == this->scalar() &&
       quat.vector() == this->vector())
   {
      return false;
   }      
   else
   {
      return true;
   }  
}

//: set self to quat
inline vjQuat& vjQuat::operator=( const vjQuat& quat )
{
   this->copy( quat );
   return *this;
}

//: set self to 'pure' quaternion pure_quat
// NOTE: a pure quaternion is one with scalar component set to 0
inline vjQuat& vjQuat::operator=( const vjVec3& pure_quat )
{
   this->copy( pure_quat );
   return *this;
}   



#endif
