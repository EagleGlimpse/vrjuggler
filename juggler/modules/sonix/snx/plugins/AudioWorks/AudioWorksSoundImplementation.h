
/****************** <SNX heading BEGIN do not edit this line> *****************
 *
 * sonix
 *
 * Original Authors:
 *   Kevin Meinert
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 *
 ****************** <SNX heading END do not edit this line> ******************/
/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000, 2001 by Iowa State University
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




/* Generated by Together */

#ifndef SNXAUDIOWORKSSOUNDIMPLEMENTATION_H
#define SNXAUDIOWORKSSOUNDIMPLEMENTATION_H
#include <iostream> // std::cout std::cerr
#include <string>
#include <gmtl/Math.h>
#include <gmtl/Matrix.h>
#include <gmtl/Vec.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/VecOps.h>
#include <gmtl/Xforms.h>

#include "snx/SoundImplementation.h"
#include "snx/SoundInfo.h"
#include "snx/SoundAPIInfo.h"

#include <PSI/aw.h> // for audioworks

namespace snx
{

class AudioWorksSoundImplementation : public snx::SoundImplementation
{
public:
   /**
    * constructor for the OpenAL implementation 
    */
   AudioWorksSoundImplementation();

   /**
    * destructor for the OpenAL implementation
    */
   virtual ~AudioWorksSoundImplementation();

   /**
     * every implementation can return a new copy of itself
     */
   virtual void clone( snx::ISoundImplementation* &newCopy );
   
   /**
    * @input alias of the sound to trigger, and number of times to play
    * @preconditions alias does not have to be associated with a loaded sound.
    * @postconditions if it is, then the loaded sound is triggered.  if it isn't then nothing happens.
    * @semantics Triggers a sound
    */
   virtual void trigger( const std::string & alias, const int & looping = 0 );

   /**
     * is the sound currently playing?
     */
   virtual bool isPlaying( const std::string& alias );
   
   /*
    * when sound is already playing then you call trigger,
    * does the sound restart from beginning?
    * (if a tree falls and no one is around to hear it, does it make sound?)
    */
   virtual void setRetriggerable( const std::string& alias, bool onOff );

   /**
    * pause the sound, use unpause to return playback where you left off...
    */
   virtual void pause( const std::string& alias );

   /**
    * resume playback from a paused state.  does nothing if sound was not paused.
    */
   virtual void unpause( const std::string& alias );
   
   /** if the sound is paused, then return true. */
   virtual bool isPaused( const std::string& alias );
         
   /**
    * ambient or positional sound.
    * is the sound ambient - attached to the listener, doesn't change volume
    * when listener moves...
    * or is the sound positional - changes volume as listener nears or retreats..
    */
   virtual void setAmbient( const std::string& alias, bool isAmbient = false );

   /** bend the pitch. */
   virtual void setPitchBend( const std::string& alias, float amount );

   /** affect volume.  set to a value between [0..1]. */
   virtual void setVolume( const std::string& alias, float amount );
   
   /** affect cutoff. 
    * set to a value between [0..1]... 1 is no change.  0 is total cutoff. 
    */
   virtual void setCutoff( const std::string& alias, float amount );
   
   /**
    * @semantics stop the sound
    * @input alias of the sound to be stopped
    */
   virtual void stop( const std::string& alias );

   /**
    * set sound's 3D position 
    */
   virtual void setPosition( const std::string& alias, float x, float y, float z );

   /**
    * get sound's 3D position
    * @input alias is a name that has been associate()d with some sound data
    * @output x,y,z are returned in OpenGL coordinates.
    */
   virtual void getPosition( const std::string& alias, float& x, float& y, float& z );

   /**
    * set the position of the listener
    */
   virtual void setListenerPosition( const gmtl::Matrix44f& mat );

   /**
    * get the position of the listener
    */
   virtual void getListenerPosition( gmtl::Matrix44f& mat );

   /**
    * start the sound API, creating any contexts or other configurations at startup
    * @postconditions sound API is ready to go.
    * @semantics this function should be called before using the other functions in the class.
    */
   virtual int startAPI();

   /**
    * kill the sound API, deallocating any sounds, etc...
    * @postconditions sound API is ready to go.
    * @semantics this function could be called any time, the function could be called multiple times, so it should be smart.
    * @return value: 1 if success, 0 otherwise
    */
   virtual void shutdownAPI();

   /**
     * query whether the API has been started or not
     * @semantics return true if api has been started, false otherwise.
     */
   virtual bool isStarted() const;

   /**
    * configure the sound API global settings
    */
   virtual void configure( const snx::SoundAPIInfo& sai );

   /**
     * configure/reconfigure a sound
     * configure: associate a name (alias) to the description if not already done
     * reconfigure: change properties of the sound to the descriptino provided.
     * @preconditions provide an alias and a SoundInfo which describes the sound
     * @postconditions alias will point to loaded sound data
     * @semantics associate an alias to sound data.  later this alias can be used to operate on this sound data.
     */
   virtual void configure( const std::string& alias, const snx::SoundInfo& description );

   /**
     * remove a configured sound, any future reference to the alias will not
     * cause an error, but will not result in a rendered sound
     */
   virtual void remove( const std::string& alias );

   /**
    * clear all associate()tions.
    * @semantics any existing aliases will be stubbed. aounds will be unbind()ed
    */
   virtual void clear();
   
   /**
    * load/allocate the sound data this alias refers to the sound API
    * @postconditions the sound API has the sound buffered.
    */
   virtual void bind( const std::string& alias );
   
   /**
    * unload/deallocate the sound data this alias refers from the sound API
    * @postconditions the sound API no longer has the sound buffered.
    */
   virtual void unbind( const std::string& alias );

   /**
    * take a time step of [timeElapsed] seconds.
    * @semantics call once per sound frame (doesn't have to be same as your graphics frame)
    * @input time elapsed since last frame
    */
   virtual void step( const float & timeElapsed );

    /** @link dependency */
    /*#  snx::SoundInfo lnkSoundInfo; */
protected:
   float mTotalTimeElapsed;
   
   class AWSoundInfo
   {
   public:
      awWave*     mWave;   // Each wave must have a sound
      awSound*    mSound;   // and vice versa
      awPlayer*   mPlayer; // the player (so we can support non-positional sounds, attach the sound to the listener)
   };   
   
   std::map<std::string, AWSoundInfo> mBindTable;

   awEngine*      mEngine;
   awChannel*     mChannel;
   awEnv*         mEnv;
   awScene*       mScene;
   awObserver*    mObs;
   
   bool mIsStarted;

   /** @link dependency */
   /*#  snx::SoundAPIInfo lnksnx::SoundAPIInfo; */

};

}; // end namespace 


#endif //SNXAUDIOWORKSSOUNDIMPLEMENTATION_H
