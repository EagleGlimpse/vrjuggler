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

#ifndef _DTK_MEMORY_SEGMENT_
#define _DTK_MEMORY_SEGMENT_

#include <vjConfig.h>
#include <Input/vjDTK/vjDTK.h>

typedef enum {
    dtk_pos_ZYXEuler = 0, 
    dtk_pos_XYZEuler,
    dtk_pos_QUAT,  
    dtk_digital, 
    dtk_analog, 
    dtk_custom
} vjDTK_memoryType;

typedef enum {
    DTK_INVALID = 0,
    DTK_INT,
    DTK_FLOAT,
    DTK_BYTES,
    DTK_CSTRING
} vjDTK_dataType;

class dtkSharedMem;
class dtkClient;

class vjDTKMemorySegment
{
public:
    vjDTKMemorySegment();
    ~vjDTKMemorySegment();
    
    //: configure the flock with a config chunk
    bool config(vjConfigChunk* c);

    //: return what chunk type is associated with this class.
    static std::string getChunkType() { return std::string("DTKMemorySegment");}

    const char* SegmentName() { return _segmentName; }
    const char* RemoteHost() { return _remotehost; }
    
    vjDTK_memoryType SegmentType() { return _segmentType; }
    
    //: Returns the number of items, size of each item, and the total memory size of
    //  The specified memory segment.  This allows users to make sure they are
    //  receiving the correct data.
    int	ItemCount() { return _numItems; }
    int ItemSize() {
	    if(_segmentSize % _numItems != 0); //ERROR!!!  Do something here!!!
	    return _segmentSize/_numItems; 
    }
    int SegmentSize() { return _segmentSize; }
    
    
    //: The index is used by the Multi type device vjDTK.  The index is used for the proxy
    //  interfaces.  The Item index is specified at configuration time, depending on the
    //  order each chunk was entered.
    int ItemIndex() { return vjInputIndex; }
    void setItemIndex(int i) { vjInputIndex = i; }
    
    //: Makes the connection to the DTK Client and gets the memory segment.
    bool connectSegment(dtkClient* in_parent);

public:

    //: Typecasting Operators
    operator float*() const;
    operator int*() const;
    operator char*() const;
    
private:
    dtkSharedMem* m;
    dtkClient* parent_client;
    
    char* _segmentName;
    char* _remotehost;

    vjDTK_memoryType _segmentType;
    vjDTK_dataType _type;

    int vjInputIndex;


    int _numItems;
    int _segmentSize;
    
    float *floatData;
    int	  *intData;
    char  *charData;
};

#endif
