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

#include <vjConfig.h>

#include <strstream>
#include <fstream>

#include <dtk.h>
#include <dlfcn.h>

#include <Utils/vjDebug.h>
#include <Config/vjConfigChunk.h>
#include <Input/vjDTK/vjDTK.h>

// Helper to return the index for theData array
// given the stationNum we are dealing with and the bufferIndex
// to get
//! ARGS:stationNum - The number of the cube we care about
//! ARGS:bufferIndex - the value of current, progress, or valid (it is an offset in the array)
// XXX: We are going to say the cubes are 0 based 


int vjDTK::getStationIndex(int stationNum, int bufferIndex)
{
    // We could assert here to check that bufferIndex is valid.  As far as station index, there are several
    // places which we have different ranges of values for it.  So we can't really assert to see if it is valid.
    int ret_val = (stationNum*3)+bufferIndex;
    return ret_val;
}


vjDTK::vjDTK()
{
    _dtkSegments = NULL;
    port = NULL;
}

bool vjDTK::config(vjConfigChunk *c)
{
    vjDEBUG(vjDBG_INPUT_MGR,1) << "	 vjDTK::config(vjConfigChunk*)"
			       << std::endl << vjDEBUG_FLUSH;

    if (!vjPosition::config(c) || !vjDigital::config(c) || !vjAnalog::config(c))
	return false;
    
    
// This is where we map the shared memory blocks to positional, digital, and analog data types
// The Config chunk will have the following items:
//     String: port
//        int: numberOfDevices
// Each Device will have the following items:
//     String: segmentName
//        int: dataType

    vjConfigChunk* embeddedChunk = NULL;
    int i = 0;
    
/* Dynamically Load the DTK Library
    void* handle;
    char *DTKFileName = c->getProperty("dlfilename").cstring();
    handle = dlopen(DTKFileName, RTLD_NOW|RTLD_GLOBAL);
    if(handle == NULL) return false;
*/   

 
// Setup each embedded memory chunk

    numSegments = c->getNum("segments");
    if(numSegments > 0)
    {
	_dtkSegments = new vjDTKMemorySegmentHandle[numSegments];
	
	for(i = 0; i < numSegments; i++)
	{
	    embeddedChunk = static_cast<vjConfigChunk*>(c->getProperty("segments", i));
	    _dtkSegments[i] = new vjDTKMemorySegment;
	    if(embeddedChunk != NULL)
	    {
		if(!(_dtkSegments[i]->config(embeddedChunk)))
		{
		    continue;//ERROR!!! Go on to next item...
		}
		switch(_dtkSegments[i]->SegmentType())
		{
		    case dtk_pos_XYZEuler:
		    case dtk_pos_ZYXEuler:
		    case dtk_pos_QUAT:
			_dtkSegments[i]->setItemIndex(numPositional++); break;
		    case dtk_digital:
			_dtkSegments[i]->setItemIndex(numDigital++); break;
		    case dtk_analog:
			_dtkSegments[i]->setItemIndex(numAnalog++); break;
		}
		
	    }
	    delete embeddedChunk; // Free the copy of the embedded memory chunk
	}
    }
    
    return true;
}

vjDTK::~vjDTK()
{
    this->stopSampling();  
    int i = 0;  
    
    if (_dtkSegments != NULL) {
	for(i = 0; i < numSegments; i++) {
	    delete _dtkSegments[i];
	}
	delete [] _dtkSegments;
	_dtkSegments = NULL;
    }
    
    if (theData != NULL)
	delete [] theData;
    if (mDataTimes != NULL)
	delete [] mDataTimes;
    if (mAnalogData != NULL)
	delete [] mAnalogData;
    if (mDigitalData != NULL)
	delete [] mDigitalData;
}

// Main thread of control for this active object
void vjDTK::controlLoop(void* nullParam)
{

    
    if (theData != NULL)
	delete [] theData;
    if (mDataTimes != NULL)
	delete [] mDataTimes;
    if (mAnalogData != NULL)
	delete [] mAnalogData;
    if (mDigitalData != NULL)
	delete [] mDigitalData;
     
    int numbuffs = numPositional*3;
    theData = (vjMatrix*) new vjMatrix[numbuffs];
    mDataTimes = new vjTimeStamp[numbuffs];

    numbuffs = numDigital*3;
    mDigitalData = new int[numbuffs];
    
    numbuffs = numAnalog*3;
    mAnalogData = new float[numbuffs];

// Reset current, progress, and valid indices
    resetIndexes();
    
// Loop through and keep sampling
    for (;;)
    {
	this->sample();
    }
}

int vjDTK::startSampling()
{
// make sure inertia cubes aren't already started
    if (this->isActive() == true)
    {
	vjDEBUG(vjDBG_INPUT_MGR,2) << "vjDTK was already started."
                                   << std::endl << vjDEBUG_FLUSH;
	return 0;
    }

// Has the thread actually started already
    if(myThread != NULL)
    {
	vjDEBUG(vjDBG_ERROR,vjDBG_CRITICAL_LVL) << clrOutNORM(clrRED,"ERROR:")
						<< "vjIsense: startSampling called, when already sampling.\n"
						<< vjDEBUG_FLUSH;
	vjASSERT(false);
	
    } else {
    	if (!this->startDTK()) {
		vjDEBUG(vjDBG_ERROR,vjDBG_CRITICAL_LVL) << clrOutNORM(clrRED,"ERROR:")
						<< "vjDTK: \n"
						<< vjDEBUG_FLUSH;
		return 0;
    	}

// Create a new thread to handle the control
	vjThreadMemberFunctor<vjDTK>* memberFunctor =
	    new vjThreadMemberFunctor<vjDTK>(this, &vjDTK::controlLoop, NULL);
	vjThread* new_thread;
	new_thread = new vjThread(memberFunctor);
	myThread = new_thread;


	if ( myThread == NULL )
	{
	    return 0;  // Fail
	} else {
	    return 1;   // success
	}
    }

    return 0;
}

int vjDTK::sample()
{
    if (this->isActive() == false)
	return 0;

    int i;
    int index;
    vjDTKMemorySegment* segment = NULL;
    float *floatData;
    int   *intData;

    vjTimeStamp sampletime;
    vjMatrix world_T_transmitter, transmitter_T_reciever, world_T_reciever;

    sampletime.set();

    vjThread::yield()

    for (i = 0; i < numSegments; i++)
    {
	segment = _dtkSegments[i];
	if(segment != NULL)    
	{    
	    if(segment->SegmentType() <= dtk_pos_QUAT && segment->ItemIndex() < numPositional) {
	        index = getStationIndex(segment->ItemIndex(),progress);
	    
	        floatData = static_cast<float*>(*segment);
            	// Check to see that the pointer is valid
            	if(floatData == NULL) continue;


	    
	        if(segment->SegmentType() != dtk_pos_QUAT)
            	{
		        // Determine which format the Euler angle is in
			if(segment->SegmentType() == dtk_pos_XYZEuler) theData[index].makeXYZEuler(floatData[5], floatData[4], floatData[3]);
		        if(segment->SegmentType() == dtk_pos_ZYXEuler) theData[index].makeZYXEuler(floatData[5], floatData[4], floatData[3]);
		
		        theData[index].setTrans(floatData[0], floatData[1], floatData[2]);
	        } else {
		        // Data is in Quaternion Format	
				
		
	        }
	        mDataTimes[index] = sampletime;

	// Transforms between the cord frames
	// See transform documentation and VR System pg 146
	// Since we want the reciver in the world system, Rw
	// wTr = wTt*tTr

	        world_T_transmitter = xformMat;                    // Set transmitter offset from local info
	        transmitter_T_reciever = theData[index];           // Get reciever data from sampled data
	        world_T_reciever.mult(world_T_transmitter, transmitter_T_reciever);   // compute total transform
	        theData[index] = world_T_reciever;                                     // Store corrected xform back into data
	    
	    } else if(segment->SegmentType() == dtk_digital && segment->ItemIndex() < numDigital) {
	    
	        index = getStationIndex(segment->ItemIndex(),progress);

	        intData = static_cast<int*>(*segment);
            	// Check to see that the pointer is valid
            	if(intData == NULL) continue;
	    
	        mDigitalData[index] = *(intData);
	    	    
            } else if(segment->SegmentType() == dtk_analog && segment->ItemIndex() < numAnalog) {
	    
	        index = getStationIndex(segment->ItemIndex(),progress);
	    
	        floatData = static_cast<float*>(*segment);
            	// Check to see that the pointer is valid
            	if(floatData == NULL) continue;
	    
	        mAnalogData[index] = *(floatData);
	    }
	}
    }
// Locks and then swaps the indices
    swapValidIndexes();

    return 1;
}

int vjDTK::stopSampling()
{
    if (this->isActive() == false)
	return 0;

    if (myThread != NULL)
    {
	vjDEBUG(vjDBG_INPUT_MGR,1) << "vjDTK::stopSampling(): Stopping the DTK thread... " 
				   << vjDEBUG_FLUSH;

	myThread->kill();
	delete myThread;
	myThread = NULL;

	this->stopDTK();

	vjDEBUG(vjDBG_INPUT_MGR,1) << "stopped." << std::endl << vjDEBUG_FLUSH;
    }

    return 1;
}

vjMatrix* vjDTK::getPosData( int d )
{
    if( (this->isActive() == false) || (d < 0) || (d >= numPositional) )
	return NULL;

    return (&theData[getStationIndex(d,current)]);
}


int vjDTK::getDigitalData( int d )
{
    if( (this->isActive() == false) || (d < 0) || (d >= numDigital) )
	return 0;
  
    return mDigitalData[getStationIndex(d,current)];
}

float vjDTK::getAnalogData( int d )
{
 
    if( (this->isActive() == false) || (d < 0) || (d >= numAnalog) )
	return 0.0;
  
    return mAnalogData[getStationIndex(d,current)];
 
}  

vjTimeStamp* vjDTK::getPosUpdateTime (int d) 
{
    if( (this->isActive() == false) || (d < 0) || (d >= numPositional) )
	return NULL;

    return (&mDataTimes[getStationIndex(d,current)]);
}

void vjDTK::updateData()
{
    if (this->isActive() == false)
	return;
				
// this unlocks when this object is destructed (upon return of the function)
    vjGuard<vjMutex> updateGuard(lock);
    
    
// TODO: modify the datagrabber to get correct data
// Copy the valid data to the current data so that both are valid
    int i;

    for(i = 0;i < numPositional; i++)
	theData[getStationIndex(i,current)] = theData[getStationIndex(i,valid)];   // first hand
    for(i = 0; i < numDigital; i++)
	mDigitalData[getStationIndex(i,current)] = mDigitalData[getStationIndex(i,valid)];
    for(i = 0; i < numDigital; i++)
	mAnalogData[getStationIndex(i,current)] = mAnalogData[getStationIndex(i,valid)];

// Locks and then swap the indicies
    swapCurrentIndexes();
}

bool vjDTK::startDTK()
{
    int i = 0;
    
    if( active || _client != NULL) return false;
    
    if( port != NULL ) _client = new dtkClient( port );
    else _client = new dtkClient();
	
    for(i = 0; i < numSegments; i++ )
    {
	    _dtkSegments[i]->connectSegment(_client);
    }
    
    active = true;
    
    return true;
}

bool vjDTK::stopDTK()
{
    if( !active || _client == NULL) return false;
    delete _client;	
    active = false;
    
    return true;
}