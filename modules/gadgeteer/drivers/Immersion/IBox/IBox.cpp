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
#include <vpr/System.h>

#include <gadget/Devices/Immersion/Ibox.h>
#include <gadget/Util/Debug.h>
#include <jccl/Config/ConfigChunk.h>

namespace gadget
{

//: Config function
// Configures the ibox
bool IBox::config(jccl::ConfigChunkPtr c)
{
  if(! (Input::config(c) && Analog::config(c) && Digital::config(c) ))
      return false;

  vprDEBUG(gadgetDBG_INPUT_MGR,3) << "   IBox::config:" << std::endl
                                  << vprDEBUG_FLUSH;
  mPortStr = static_cast<std::string>(c->getProperty( "port" ));

  // Done in Input
  //active = 0;
  mBaudRate = (long) static_cast<int>(c->getProperty("baud"));

  vprDEBUG(gadgetDBG_INPUT_MGR,1)
     << "   Creating an IBox.. params: " << std::endl
     << "    portnum: " << mPortStr << std::endl
     << "        baud   : " << mBaudRate << std::endl
     << "   instanceName: " << mInstName << std::endl << std::endl
     << vprDEBUG_FLUSH;

  return true;
}

/**********************************************************
  IBox::~IBox()

  IBox Destructor, just stop sampling, let other destructors
  do the rest.

*********************************************** ahimberg */
IBox::~IBox()
{
  stopSampling();
}

/**********************************************************
  int IBox::startSampling()

  Starts the Ibox sampling, spawns a thread to query for
  data.

  returns 1 if successful, 0 if it failed or was already sampling

*********************************************** ahimberg */
int IBox::startSampling()
{
   ibox_result result;

   if (mThread == NULL)
   {
      result = mPhysicalIbox.connect(mPortStr, mBaudRate);
      if (result == mPhysicalIbox.SUCCESS)
      {
         mActive = true;
         vprDEBUG(gadgetDBG_INPUT_MGR,1) << "     Connected to IBox.\n"
                                         << std::flush << vprDEBUG_FLUSH;
      } else
      {
         mActive = false;
         vprDEBUG(gadgetDBG_INPUT_MGR,0)
            << "   FAILED TO CONNECT to the Ibox named " << mInstName
            << std::endl << "     Ibox settings were: " << std::endl
            << "      port : " << mPortStr << std::endl
            << "   mBaudRate: " << mBaudRate << std::endl << std::endl
            << vprDEBUG_FLUSH;
         return 0;
      }
      mPhysicalIbox.std_cmd(0,0,0);


      IBox* devicePtr = this;
      void sampleBox(void*);
      mThread = new vpr::Thread(sampleBox, (void*)devicePtr);
      if (!mThread->valid())
         return 0; //fail
      else
      {
         return 1;
      }
   }
   else
      return 0; // already sampling
}

/**********************************************************
  void sampleBox(void*)

  The spawned thread just loops from here

*********************************************** ahimberg */
void sampleBox(void* pointer)
{
   IBox* devPointer = (IBox*) pointer;

   for (;;)
     devPointer->sample();

}

/**********************************************************
  int IBox::sample()

  IBox Sampler function, tries to get a packet of new stuff,
  when it does it fills up the data and swaps swaps the
  progess/valid array indicies.

  Each call to this function is not guaranteed to result in new data.

*********************************************** ahimberg */
int IBox::sample()
{
   //struct timeval tv;
   // double start_time, stop_time;
   ibox_result result;
   //int tmp;
   //static int c = 0;
   IboxData cur_reading;

   result = mPhysicalIbox.check_packet();
   if (result == mPhysicalIbox.NO_PACKET_YET)
   {
   }
   else if (result == mPhysicalIbox.SUCCESS)
   {
      mPhysicalIbox.std_cmd(0,0,0);
      //    if (c == 0) {
      //      vpr::System::gettimeofday(&tv,0);
      //      start_time = (double)tv.tv_sec+ (double)tv.tv_usec / 1000000.0;
      //    }
      //    c++;
      //    if (c == 60) {
      //      vpr::System::gettimeofday(&tv,0);
      //      stop_time = (double)tv.tv_sec+ (double)tv.tv_usec / 1000000.0;
      //      std::cout << 1/((stop_time-start_time) / 60)
      //                << "  " << std::endl;
      //      c = 0;
      //    }

      cur_reading.button[0] = mPhysicalIbox.buttonFunc(0);
      cur_reading.button[1] = mPhysicalIbox.buttonFunc(1);
      cur_reading.button[2] = mPhysicalIbox.buttonFunc(2);
      cur_reading.button[3] = mPhysicalIbox.buttonFunc(3);

      // we really need to do normalization here instead of in getData.
      // need this so we return consistent AnalogData. -cj
      float f;
      this->normalizeMinToMax( mPhysicalIbox.analogFunc(0), f );
      cur_reading.analog[0] = f;
      this->normalizeMinToMax( mPhysicalIbox.analogFunc(1), f );
      cur_reading.analog[1] = f;
      this->normalizeMinToMax( mPhysicalIbox.analogFunc(2), f );
      cur_reading.analog[2] = f;
      this->normalizeMinToMax( mPhysicalIbox.analogFunc(3), f );
      cur_reading.analog[3] = f;

      cur_reading.setTime();

      //swapValidIndexes();     // Swap the buffers since we just read in a complete value
      mDigitalSamples.lock();
      mAnalogSamples.lock();
      mDigitalSamples.addSample(cur_reading.button);
      mAnalogSamples.addSample(cur_reading.analog);
      mAnalogSamples.unlock();
      mDigitalSamples.unlock();
   }
   return 1;
}

/**********************************************************
  int IBox::stopSampling()

  Kill the sampling thread if it exists, disconnect from
  the ibox.

*********************************************** ahimberg */
int IBox::stopSampling()
{
  if (mThread != NULL)
  {
    mThread->kill();
    delete(mThread);
    mThread = NULL;

    vpr::System::usleep(100);        // 100 uSec pause

    mPhysicalIbox.disconnect();
    std::cout << "stopping the ibox.." << std::endl;

  }
  return 1;
}

/**********************************************************
  void IBox::updateData()

  Swap the current/valid array indicies

*********************************************** ahimberg */
void IBox::updateData()
{
   mAnalogSamples.swapBuffers();
   mDigitalSamples.swapBuffers();
   return;
}

};
