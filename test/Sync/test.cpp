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

/// TOP OF TEST_CPP

#include <iostream>
#include <stdio.h>
#include <ulocks.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#include <sprocGroup.h>
#include <Sync/vjMutex.h>
#include <Sync/vjSemaphore.h>
#include <Sync/NullMutex.h>

void doIt(void*);
void printArray(void);

long counter;
//NullMutex myMutex;
vjMutex* myMutex;
vjSemaphore* mySemaphore;

///---//  Beginning of main
void main()
{
    mySemaphore = new vjSemaphore;
    myMutex = new vjMutex;
    //myMutex = new NullMutex;
    std::cout << "Hello there\n\n" << std::flush;
    std::cout << "Start:\n\n";

    counter = 0;
    std::cout << "Counter: " << counter << std::endl;
    
    
    sprocInit(NULL);
    sginap(100);	

//-------------------------------------------------
	    // Timing variables
    struct timeval startTime, endTime;		    // Used to calculate time
    double startSecond, endSecond, totalSeconds;    // performance
    
    gettimeofday(&startTime, NULL);	    // Get the starting time
//--------------------------------------------------
    
    for (float i=0;i<10000;i++) {
	sprocFunc(doIt,NULL,NULL);      
    }
    sprocBarrier();
    
//--------------------------------------------------------
    gettimeofday(&endTime, NULL);	    // Get ending time
    startSecond = (startTime.tv_sec + (startTime.tv_usec * 0.000001));
    endSecond = endTime.tv_sec + (endTime.tv_usec * 0.000001);
    totalSeconds = (endSecond - startSecond);
    
    std::cout << "\nDone: It took... " << totalSeconds << " seconds"
              << std::flush;
//---------------------------------------------------------
    
    sginap(100);
    std::cout << "\n\nCounter: " << counter << std::endl << std::flush;
    
    std::cout << "End:\n\n";
    std::cout << "\n" << std::flush;
    
    // -- Clean up --- //
    delete mySemaphore;
    delete myMutex;    
}

void doIt(void* param)
{
///---//  Beginning of doIt

    //mySemaphore->acquire();
    //myMutex.acquire();
    mySemaphore->acquire();
	int tempCounter = counter;
	
	for(int j=0;j<1000;j++)
	    counter++; 
	
	counter = tempCounter + 1;
	//cerr << "Counter now: " << counter << endl << flush;
    mySemaphore->release();
    //myMutex.release();
    //mySemaphore->release();
}

