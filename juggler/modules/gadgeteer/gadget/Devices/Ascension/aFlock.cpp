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
#include <iostream>        // for std::cout
#include <fstream>         // for std::ifstream
#include <string.h>

#include <unistd.h>        // for sleep, and ioctl
#include <assert.h>        // for assert

#include <gadget/Devices/Ascension/aFlock.h>

const int aFlock::MAXCHARSTRINGSIZE = 256;


//: Configure Constructor
// Give:                                                 <BR>
//   port - such as "/dev/ttyd3"                         <BR>
//   baud - such as 38400, 19200, 9600, 14400, etc...    <BR>
//   sync - sync type.                                   <BR>
//   block - blocking                                    <BR>
//   numBrds - number of birds in flock,                 <BR>
//   transmit - transmitter unit number,                 <BR>
//   hemi - hemisphere to track from,                    <BR>
//   filt - filtering type,                              <BR>
//   report -                                            <BR>
//   calfile - a calibration file, if "", then use none. <BR>
//                                                       <BR>
// Result: configures internal data members,
//         doesn't actually talk to the FOB yet.
aFlock::aFlock(const char* const port,
		const int& baud,
		const int& sync,
		const int& block,
		const int& numBrds,
		const int& transmit,
		const BIRD_HEMI& hemi,
		const BIRD_FILT& filt,
		const char& report,
		const char* const calfile) :
		  _reportRate(report),
		  _hemisphere(hemi),
		  _filter( filt ),
		  _baud(baud),
		  _syncStyle(sync),
		  _blocking(block),
		  _numBirds(numBrds),
		  _xmitterUnitNumber(transmit),
		  _usingCorrectionTable(false),
		  _active(false),
		  mSleepFactor(3)
{
  std::cout << "mSleepFactor = " << mSleepFactor<< std::endl;
  if (port != NULL) {
    strncpy(_port, port, aFlock::MAXCHARSTRINGSIZE);
    _serial_port = new vpr::SerialPort(_port);
  }
  else {
    _serial_port = NULL;
  }

  // fix the report rate if it makes no sense.
  if ((_reportRate != 'Q') && (_reportRate != 'R') &&
      (_reportRate != 'S') && (_reportRate != 'T'))
  {
     // illegal report rate, defaulting to "every other cycle" (R)
     assert(false);
     _reportRate = 'R';
  }

  if (calfile != NULL && calfile[0] != '\0')
  {
  	strncpy( _calibrationFileName, calfile, aFlock::MAXCHARSTRINGSIZE );
  	this->initCorrectionTable(_calibrationFileName);
  	_usingCorrectionTable = true;
  }	
}

//: Destructor
aFlock::~aFlock()
{
    this->stop();

    if ( _serial_port != NULL ) {
        delete _serial_port;
        _serial_port = NULL;
    }
}

//: see if the flock is active or not
const bool& aFlock::isActive() const
{
	return _active;
}

//: set the port to use
//  this will be a string in the form of the native OS descriptor <BR>
//  ex: unix - "/dev/ttyd3", win32 - "COM3" <BR>
//  NOTE: flock.isActive() must be false to use this function
void aFlock::setPort(const char* const serialPort)
{
    if (_active)
    {
	std::cout << "Flock: Cannot change the serial Port while active\n";
	return;
    }

    if ( serialPort != NULL ) {
        strncpy(_port, serialPort, aFlock::MAXCHARSTRINGSIZE);

        if ( _serial_port != NULL ) {
            delete _serial_port;
        }

        _serial_port = new vpr::SerialPort(_port);
    }
}

//: get the port used
//  this will be a string in the form of the native OS descriptor <BR>
//  ex: unix - "/dev/ttyd3", win32 - "COM3"
const char* aFlock::getPort() const
{
    return _port;
}

//: set the baud rate
//  this is generally 38400, consult flock manual for other rates. <BR>
//  NOTE: flock.isActive() must be false to use this function
void aFlock::setBaudRate(const int& baud)
{
    if (_active)
    {
	std::cerr << "Flock: Cannot change the baud rate while active\n";
	return;
    } else {
	_baud = baud;
    }
}

void aFlock::check_known(){
	char exam1[1], exam2[1];
	exam1[0] = 'O';
	exam2[0] = 0xF;
	vpr::Uint32 buf=0, buf1=0;
	char in[2];
	in[0]='a';

	
	while(true){
		_serial_port->write(exam1, sizeof(char), buf);
		_serial_port->write(exam2, sizeof(char), buf);

		_serial_port->readn(in, sizeof(char), buf1);
		std::cout << in[0] << " " << buf << " " << buf1 << std::endl;
	}
	
}

void aFlock::check_group(){
	char exam1[1], exam2[1];
	exam1[0] = 'O';
	exam2[0] = 0x23;
	vpr::Uint32 buf=0, buf1=0;
	char in[2];
	in[0]='a';

	
	while(true){
		_serial_port->write(exam1, sizeof(char), buf);
		_serial_port->write(exam2, sizeof(char), buf);

		_serial_port->readn(in, sizeof(char), buf1);
		std::cout << in[0] << " " << buf << " " << buf1 << std::endl;
	}
	
}
	

	


//: call this to connect to the flock device.
//  NOTE: flock.isActive() must be false to use this function
int
aFlock::start() {
    int retval;
    if (!_active) {
        if (open_port() == -1 ) {
           retval = 0;
        }
        else {
            std::cout << "[aFlock] Setting blocking state ("
                      << (_blocking ? "" : "non-") << "blocking)\n"
                      << std::flush;
            set_blocking();

	    std::cout << "[aFlock] Setting hardware flow control\n" << std::flush;
            disable_hardware_flow_control(); 
	    
	    std::cout << "[aFlock] Setting sync\n" << std::flush;
            set_sync();

	    
            std::cout << "[aFlock] Setting group\n" << std::flush;
            set_group();

//	    check_known();
//	    check_group();
	    
            std::cout << "[aFlock] Setting autoconfig\n" << std::flush;
            set_autoconfig();
         
//	    check_config(); 
   
            std::cout << "[aFlock] Setting transmitter\n" << std::flush;
            set_transmitter();

	    
            std::cout << "[aFlock] Setting filter\n" << std::flush;
            set_filter();
	    
            std::cout << "[aFlock] Setting hemisphere\n" << std::flush;
            set_hemisphere();

            std::cout << "[aFlock] Setting pos_angles\n" << std::flush;
            set_pos_angles();

	    check_pos_angles();
	    
            std::cout << "[aFlock] Setting pickBird\n" << std::flush;
            pickBird(_xmitterUnitNumber);

            std::cout << "[aFlock] Setting rep_and_stream\n" << std::flush;
            set_rep_and_stream();

	    check_rep_and_stream();

            std::cout  << "[aFlock] Ready to go!\n\n" << std::flush;

            // flock is active.
            _active = true;

            // return success
            retval = 1;
        }
    }
    else {
        retval = 0; // already sampling
    }

    return retval;
}

//: call this repeatedly to update the data from the birds.
//  NOTE: flock.isActive() must be true to use this function
int aFlock::sample()
{

     // can't sample when not active.
     assert( _active == true );
     int i;
     int loopCount = _numBirds + 1;
     if (_xmitterUnitNumber <= _numBirds) {loopCount++;} 

     // for [1..n] birds, get their reading:
     int j = 0;

     for (i=1; i < loopCount && i < MAX_SENSORS; i++)
	{
		j++;
// If the transmitter number is less than or equal to the number of birds, we need to ignore it.

          if (i == _xmitterUnitNumber)
	  { 
                j--;
		continue;
          }

// However, we need to still copy the data into consecutive values in the wrapper class, so we
// introduce "j" to account for that correction.  It is equal to "i" while we haven't encountered
// the transmitter, but equal to "i-1" afterwards.


	// you can never go above the maximum number of sensors.
	assert( i < MAX_SENSORS );

	getReading(i, xPos(j), yPos(j), zPos(j), zRot(j), yRot(j), xRot(j));

	if (_usingCorrectionTable)
	{
	    this->positionCorrect( this->xPos(j),
			    this->yPos(j),
			    this->zPos(j) );
	}

     }


     return 1;
}

//: stop the flock
int
aFlock::stop () {
    int retval;
    vpr::Uint32 written;
    if ( _serial_port != NULL ) {
        char bird_command[4];

        std::cout << "[aFlock] Stopping the flock ...\n" << std::flush;

        bird_command[0] = 'B';
        _serial_port->write(bird_command, 1, written);
        _serial_port->flushQueue(vpr::SerialTypes::IO_QUEUES);
        usleep(500 * mSleepFactor);

        bird_command[0] = 'G';
        _serial_port->write(bird_command, 1, written);
        _serial_port->flushQueue(vpr::SerialTypes::IO_QUEUES);
        usleep(200 * aFlock::mSleepFactor);

        _serial_port->close();

        // flock is not active now.
        _active = false;

        std::cout << "[aFlock] Stopped." << std::endl << std::flush;

        retval = 1;
    }
    else {
        retval = 0;
    }

    return retval;
}

//: Set the hemisphere that the transmitter transmits from.
//  NOTE: flock.isActive() must be false to use this function
void aFlock::setHemisphere( const BIRD_HEMI& h )
{
    if (_active)
    {
	std::cout << "Flock: Cannot change the hemisphere\n" << std::flush;
	return;
    } else {
	// Set it.
        _hemisphere = h;
    }
}

//: Set the type of filtering that the flock uses
//  NOTE: flock.isActive() must be false to use this function
void aFlock::setFilterType( const BIRD_FILT& f )
{
    if (_active)
    {
	std::cout << "Flock: Cannot change filter type while active\n"
                  << std::flush;
	return;
    } else {
	// Set it.
	_filter = f;
    }
}

//: Set the report rate that the flock uses
//  NOTE: flock.isActive() must be false to use this function
void aFlock::setReportRate( const char& rRate )
{
    if (_active)
    {
	std::cout << "Flock: Cannot change report rate while active\n"
                  << std::flush;
	return;
    } else {
	// Set it.
	_reportRate = rRate;
    }
}

//: Set the unit number of the transmitter
//  give - an integer that is the same as the dip switch
//         setting on the transmitter box (for the unit number) <BR>
//  NOTE: flock.isActive() must be false to use this function
void aFlock::setTransmitter( const int& Transmit )
{
  if (_active)
  {
      std::cout << "Flock: Cannot change transmitter while active\n"
                << std::flush;
      return;
  } else {
      // Set it.
      _xmitterUnitNumber = Transmit;
  }
}

//: Set the number of birds to use in the flock.
//  give - an integer number not more than the number of
//         birds attached to the system <BR>
//  NOTE: flock.isActive() must be false to use this function
void aFlock::setNumBirds( const int& n )
{
    if (_active)
    {
	std::cout << "Flock: Cannot change num birds while active\n"
                  << std::flush;
	return;
    } else {
	// Set it.
	_numBirds = n;
    }
}

//: set the video sync type
//  this option allows the Flock to syncronize its pulses with
//  your video display.  This will eliminate most flicker caused
//  by the magnetic distortion. <BR>
//  - Refer to your flock manual for what number to use.
//  NOTE: flock.isActive() must be false to use this function
void aFlock::setSync(const int& sync)
{
  if (_active)
  {
      std::cout << "Flock: Cannot change report rate while active\n"
                << std::flush;
      return;
  } else {
      // Set it.
      _syncStyle = sync;
  }
}

//: set blocking of flock
//  see flock manual for details.
//  NOTE: flock.isActive() must be false to use this function
void aFlock::setBlocking( const int& blVal )
{
  if (_active)
  {
      std::cout << "Flock: Cannot change blocking while active\n" << std::flush;
      return;
  } else {
      // Set it.
      _blocking = blVal;
  }
}

//: with the calibration table info, correct a given position
//  give - a position in x, y, z euclidian coordinates
//  returns - a corrected position in x, y, z euclidian coordinates
void aFlock::positionCorrect( float& x, float& y, float& z )
{
    int xlo,ylo,zlo,xhi,yhi,zhi;
    float a,b,c,a1,b1,c1;

    // Find corners
    xlo = (int)(x-caltable.xmin);
    ylo = (int)(y-caltable.ymin);
    zlo = (int)(z-caltable.zmin);
    xhi = xlo+1;
    yhi = ylo+1;
    zhi = zlo+1;
    a1 = 1 - (a = caltable.xmin + xhi - x);
    b1 = 1 - (b = caltable.ymin + yhi - y);
    c1 = 1 - (c = caltable.zmin + zhi - z);

    x = a * b* c*caltable.xloc[xlo][ylo][zlo] +
        a * b*c1*caltable.xloc[xlo][ylo][zhi] +
        a *b1* c*caltable.xloc[xlo][yhi][zlo]+
        a *b1*c1*caltable.xloc[xlo][yhi][zhi]+
        a1* b* c*caltable.xloc[xhi][ylo][zlo]+
        a1* b*c1*caltable.xloc[xhi][ylo][zhi]+
        a1*b1* c*caltable.xloc[xhi][yhi][zlo]+
        a1*b1*c1*caltable.xloc[xhi][yhi][zhi];
    y = a * b* c*caltable.yloc[xlo][ylo][zlo] +
        a * b*c1*caltable.yloc[xlo][ylo][zhi] +
        a *b1* c*caltable.yloc[xlo][yhi][zlo]+
        a *b1*c1*caltable.yloc[xlo][yhi][zhi]+
        a1* b* c*caltable.yloc[xhi][ylo][zlo]+
        a1* b*c1*caltable.yloc[xhi][ylo][zhi]+
        a1*b1* c*caltable.yloc[xhi][yhi][zlo]+
        a1*b1*c1*caltable.yloc[xhi][yhi][zhi];
    z = a * b* c*caltable.zloc[xlo][ylo][zlo] +
        a * b*c1*caltable.zloc[xlo][ylo][zhi] +
        a *b1* c*caltable.zloc[xlo][yhi][zlo]+
        a *b1*c1*caltable.zloc[xlo][yhi][zhi]+
        a1* b* c*caltable.zloc[xhi][ylo][zlo]+
        a1* b*c1*caltable.zloc[xhi][ylo][zhi]+
        a1*b1* c*caltable.zloc[xhi][yhi][zlo]+
        a1*b1*c1*caltable.zloc[xhi][yhi][zhi];
    return;
}

//: init the correction table from a file
//  give - a file name of the calibration file.
//  result - initializes the correction table with the file's info
void aFlock::initCorrectionTable( const char* const fName )
{
  int i,j,k, xsize,ysize,zsize;
  float dump;
  std::ifstream inFile;

  std::cout << "	  Initializing calibration table ... " << std::endl
            << "	    " << fName << std::endl << std::flush;
  inFile.open( fName );
  if (!inFile)
  {
	std::cout << "Unable to open calibration.table\n" << std::flush;
        return;
  } else {
     std::cout << "   Calibration table opened sucessfully." << std::endl
               << std::flush;
  }

  inFile >> caltable.xmin >> caltable.xmax
         >> caltable.ymin >> caltable.ymax
         >> caltable.zmin >> caltable.zmax;

  xsize = (int) (caltable.xmax - caltable.xmin) + 1;
  ysize = (int) (caltable.ymax - caltable.ymin) + 1;
  zsize = (int) (caltable.zmax - caltable.zmin) + 1;

  for (i = 0; i < xsize; i++)
    for (j = 0; j < ysize; j++)
      for (k = 0; k < zsize; k++)
         {
	    inFile >> dump >> dump >> dump
		   >> caltable.xloc[i][j][k]
		   >> caltable.yloc[i][j][k]
		   >> caltable.zloc[i][j][k];
	 }

   inFile.close();
}

 float& aFlock::xPos( const int& i )  { assert( i < MAX_SENSORS ); return _position[i][0]; }
 float& aFlock::yPos( const int& i )  { assert( i < MAX_SENSORS ); return _position[i][1]; }
 float& aFlock::zPos( const int& i )  { assert( i < MAX_SENSORS ); return _position[i][2]; }
 float& aFlock::zRot( const int& i )  { assert( i < MAX_SENSORS ); return _orientation[i][0]; }
 float& aFlock::yRot( const int& i )  { assert( i < MAX_SENSORS ); return _orientation[i][1]; }
 float& aFlock::xRot( const int& i )  { assert( i < MAX_SENSORS ); return _orientation[i][2]; }


///////////////////////////////////////////////////////////////////
// Local functions to aFlock.cpp
//////////////////////////////////////////////////////////////////
//: get a reading
//  give - n:    the bird unit number  <BR>
//  give - port: the flock port number <BR>
//  give - xyz positions               <BR>
//  give - zyx rotations
int
aFlock::getReading (const int& n, float& xPos, float& yPos, float& zPos,
                    float& zRot, float& yRot, float& xRot)
{
    int addr;
    vpr::Uint32 num_read=0;
    if ( _serial_port != NULL ) {
        char buff[12], group[2];
        int  c, i;
        vpr::Uint64 num=100;
        int timeO=false;

        do {
		
                if(_serial_port->readn(group, 1, num_read, vpr::Interval(num, vpr::Interval::Msec)).Timeout){
		    timeO=true;
		}
		
		std::cout << group[0] << " " << n << std::endl;
           
           

            while( i != 12) {
                if(_serial_port->readn(&buff[i], 1, num_read, vpr::Interval(num, vpr::Interval::Msec)).Timeout){
		    timeO=true;
		    i=11;
		}
                i ++;
            }

            if(_serial_port->readn(&group, 1,num_read, vpr::Interval(num, vpr::Interval::Msec)).Timeout){
		    timeO=true;
	    }


 

            addr = group[0];
        } while ( !timeO && addr != n );

        std::cout << "addr: " << addr << std::endl;

        // Position
        xPos = rawToFloat(buff[1], buff[0]) * POSITION_RANGE;
        yPos = rawToFloat(buff[3], buff[2]) * POSITION_RANGE;
        zPos = rawToFloat(buff[5], buff[4]) * POSITION_RANGE;

//        std::cerr << "Pos: " << xPos << " : " << yPos << " : " << zPos << std::endl;

        // Orientation
        zRot = rawToFloat(buff[7], buff[6])   * ANGLE_RANGE;
        yRot = rawToFloat(buff[9], buff[8])   * ANGLE_RANGE;
        xRot = rawToFloat(buff[11], buff[10]) * ANGLE_RANGE;
    }
    else {
        addr = -1;
    }

    return addr;
}





float
aFlock::rawToFloat (char& r1, char& r2) {
   // return ((float) (((r1 & 0x7f) << 9) | (r2 & 0x7f) << 2) / 0x7fff);

   short int ival1,ival2,val;
   ival1 = r1 & 0x7f;
   ival2 = r2 & 0x7f;
   val = (ival1 << 9) | ival2<<2;
   return ((float)val) / 0x7fff;
}

void
aFlock::pickBird (const int birdID) {
    vpr::Uint32 written;
    if ( _serial_port != NULL ) {
        char buff = 0xF0 + birdID;

        if(_serial_port->write(&buff, sizeof(buff), written).success()){
		std::cout << "success\n";
	}else{
		std::cout << "failure\n";
	}
        _serial_port->flushQueue(vpr::SerialTypes::IO_QUEUES);

        usleep(100 * mSleepFactor);
    }
}

//: Open the port.
//  give - a serial port
//  give - a baud rate
//  returns portId twice (created by the open function)
//  NOTE: portId is returned from both ends of this function.
//  if portId == -1 then function failed to open the port.
int
aFlock::open_port () {
    int retval;

    ///////////////////////////////////////////////////////////////////
    // Open and close the port to reset the tracker, then
    // Open the port
    ///////////////////////////////////////////////////////////////////
    if ( _serial_port != NULL ) {
        _serial_port->setOpenReadWrite();
        _serial_port->setOpenNonBlocking();
        if ( ! _serial_port->open().success() ) {
            std::cerr << "[aFlock] Port reset failed (because port open "
                      << "failed)\n";
            retval = -1;
//	    _serial_port->enableBlocking();
	}
        else {
            sleep(2);
            _serial_port->close();
	    
            std::cout << "[aFlock] Port reset successfully (port was opened "
                      << "then closed)\n" << std::flush;

            if ( ! _serial_port->open().success() ) {
                std::cerr << "[aFlock] Port open failed\n";
                retval = -1;
            }
            else {
                std::cout << "[aFlock] Port opened successfully\n"
                          << std::flush;
		std::cout << "[aFlock] Setting Port update action: ";
                _serial_port->setUpdateAction(vpr::SerialTypes::NOW);
			std::cout << "success\n";
			
                // Setup the port.
	
		std::cout << "[aFlock] setting flow control \n";
		_serial_port->controlFlow(vpr::SerialTypes::OUTPUT_ON);
		_serial_port->controlFlow(vpr::SerialTypes::INPUT_ON);
		
                std::cout << "[aFlock] Setting new baud rate at " << _baud
                          << " bits per second.\n [aFlock] Input: " << std::flush;
                if (_serial_port->setInputBaudRate(_baud).success()){
			std::cout << "success\n";
		}else{
			std::cout << "failure!\n";
		}
                
		std::cout << "[aFlock] Output: " << std::flush;
                if (_serial_port->setOutputBaudRate(_baud).success()){
			std::cout << "success\n";
		}else{
			std::cout << "failuer!\n";
		}
                std::cout << "[aFlock] Setting character size: " << std::flush;
                if( _serial_port->setCharacterSize(vpr::SerialTypes::CS_BITS_8).success()){
			std::cout << "success\n";
		}else{
			std::cout << "failure!\n";
		}
/*
                std::cout << "Setting local attachment\n" << std::flush;
                _serial_port->enableLocalAttach();
*/
                std::cout << "Setting read stuff: " << std::flush;
                if (_serial_port->enableRead().failure()) retval = -1;
            }    
        }
    }
    else{
        std::cerr << "[aFlock] ERROR -- No port has been set in aFlock::open_port()!\n";
        retval = -1;
    }

    return retval;
}

void
aFlock::set_blocking () {
    //////////////////////////////////////////////////////////////////
    // Setup a non/blocked port & Flush port
    //////////////////////////////////////////////////////////////////
    if ( _serial_port != NULL ) {
        if ( _blocking ) {
            _serial_port->enableBlocking();
            std::cerr << "enable Blocking" << std::endl;
        }
        else {
            _serial_port->enableNonBlocking();
        }

        _serial_port->flushQueue(vpr::SerialTypes::IO_QUEUES);

        usleep(1000 * mSleepFactor);
    }
}


 


void
aFlock::set_sync () {
    /////////////////////////////////////////////////////////////////
    // Set CRT sync: (manual page 82)
    //   set crt sync
    //   nosync    -   0
    //   > 72Hz    -   1 (type 1)
    //                 2 (type 2)
    /////////////////////////////////////////////////////////////////
    vpr::Uint32 written;
    if ( _serial_port != NULL ) {
        unsigned char buff[4] = {'A', 1};
        buff[1] = _syncStyle;

        if(_serial_port->write(buff, 2, written).success()){
		std::cout << "success\n";
	}else{
		std::cout << "failure\n";
	}
        _serial_port->flushQueue(vpr::SerialTypes::IO_QUEUES);
    }
}


void
aFlock::set_hemisphere() {
    /////////////////////////////////////////////////////////////////
    // Set Hemisphere for birds taking input
    //
    //  buff   [1]   [2]
    // Front: 0x00, 0x00
    // Aft  : 0x00, 0x01
    // Upper: 0x0C, 0x01
    // Lower: 0x0C, 0x00
    // Left : 0x06, 0x01
    // Right: 0x06, 0x00
    /////////////////////////////////////////////////////////////////
    vpr::Uint32 written;
    if ( _serial_port != NULL ) {
        char buff[3];

        buff[0] = 'L';

        for ( int i = 1; i < (_numBirds + 1); i++ ) {
            // Skip the transmitter.
            if ( i == _xmitterUnitNumber ) {
                continue;
            }

            pickBird(i);

            switch (_hemisphere) {
              case FRONT_HEM:
                buff[1] = 0x00;
                buff[2] = 0x00;
                break;
              case AFT_HEM:
                buff[1] = 0x00;
                buff[2] = 0x01;
                break;
              case UPPER_HEM:
                buff[1] = 0x0C;
                buff[2] = 0x01;
                break;
              case LOWER_HEM:
                buff[1] = 0x0C;
                buff[2] = 0x00;
                break;
              case LEFT_HEM:
                buff[1] = 0x06;
                buff[2] = 0x01;
                break;
              case RIGHT_HEM:
                buff[1] = 0x06;
                buff[2] = 0x00;
                break;
            }

            if(_serial_port->write(buff, sizeof(buff), written).success()){
		    std::cout << "success\n";
	    }else{
		    std::cout << "failure\n";
	    }
            _serial_port->flushQueue(vpr::SerialTypes::IO_QUEUES);

            usleep(500 * mSleepFactor);
        }
    }
}

void
aFlock::set_rep_and_stream () {
    vpr::Uint32 written;
    if ( _serial_port != NULL ) {
        char buff[1];
        /////////////////////////////////////////////////////////////////
        // Set report rate
        //             Q  Every cycle
        //  buff[0] - 'R' Every other bird cycle
        //             S  every 8 cycles
        //             T  every 32 cycles
        /////////////////////////////////////////////////////////////////
        buff[0] = _reportRate;
        if(_serial_port->write(buff, sizeof(buff), written).success()){
		std::cout << "success\n";
	}else{
		std::cout << "failure\n";
	}
        _serial_port->flushQueue(vpr::SerialTypes::IO_QUEUES);
        usleep(2000 * mSleepFactor);

        ////////////////////////////////////////////////////////////////
        // set stream mode
        ////////////////////////////////////////////////////////////////
        buff[0] = '@';
        if(_serial_port->write(buff, sizeof(buff), written).success()){
		std::cout << "success\n";
	}else{ 
		std::cout << "failure\n";
	}
        _serial_port->flushQueue(vpr::SerialTypes::IO_QUEUES);
        usleep(500 * mSleepFactor);
    }
}

void
aFlock::set_pos_angles () {
    //////////////////////////////////////////////////////////////////
    // Set Position Angles
    /////////////////////////////////////////////////////////////////
    vpr::Uint32 written;
    if ( _serial_port != NULL ) {
        char buff[1];

        for ( int i = 1; i < (_numBirds + 1); i++ ) {
            // Skip the transmitter unit.
            if ( i == _xmitterUnitNumber ) {
                continue;
            }

            pickBird(i);

            buff[0] = 'Y';
            if(_serial_port->write(buff, sizeof(buff), written).success()){
		    std::cout << "success\n";
	    }else{
		    std::cout << "failure\n";
	    }
            _serial_port->flushQueue(vpr::SerialTypes::IO_QUEUES);

            usleep(500 * mSleepFactor);
        }
    }
}

void
aFlock::set_filter () {
    ///////////////////////////////////////////////////////////////
    // Turn filters on (manual page 48)
    // 0s turn AC NARROW notch filter ON
    //         AC WIDE notch filter ON
    //         DC filter ON
    ///////////////////////////////////////////////////////////////
    vpr::Uint32 written;
    if ( _serial_port != NULL ) {
        char buff[4];

        buff[0] = 'P';
        buff[1] = 0x04;
        buff[2] = 0x00;
        buff[3] = 0;

        if(_serial_port->write(buff, sizeof(buff), written).success()){
		std::cout << "success\n";
	}else{
		std::cout << "failure\n";
	}
        _serial_port->flushQueue(vpr::SerialTypes::IO_QUEUES);

        //TODO: Do I need to sleep here?
        usleep(12000 * mSleepFactor);
    }
}

void
aFlock::set_transmitter () {
    ///////////////////////////////////////////////////////////////
    // Sets up the device for Transmitting (manual page 67)
    // Command (0x30) for Next Transmitter
    ///////////////////////////////////////////////////////////////
    vpr::Uint32 written;
    if ( _serial_port != NULL ) {
        char buff[2];

        buff[0] = (unsigned char) (0x30);
        buff[1] = (unsigned char) _xmitterUnitNumber << 4;

        if(_serial_port->write(buff, sizeof(buff), written).success()){
		std::cout << "success\n";
	}else{
		std::cout << "failure\n";
	}
        _serial_port->flushQueue(vpr::SerialTypes::IO_QUEUES);

        usleep(12000 * aFlock::mSleepFactor);
    }
}


void
aFlock::set_autoconfig () {
    ///////////////////////////////////////////////////////////////
    // FBB AUTO-CONFIGURATION (manual page 60)
    //
    // Must wait 300 milliseconds before and after this command
    ///////////////////////////////////////////////////////////////
    sleep(2);
    vpr::Uint32 written;
    if ( _serial_port != NULL ) {
        char buff[3];

        buff[0] = 'P';
        buff[1] = 0x32;
        buff[2] = _numBirds + 1;  //number of input devices + 1 for transmitter
        if(_serial_port->write(buff, sizeof(buff), written) == vpr::ReturnStatus::Succeed) std::cout << "succed" << std::endl;

        _serial_port->flushQueue(vpr::SerialTypes::IO_QUEUES);
        sleep(2);
    }
}





void
aFlock::set_group () {
    ////////////////////////////////////////////////////////////////
    // Setup group mode: (manual page 59)
    // 'P' Change Parameter
    // Number 35 (hex 23),
    // Set flag to 1 enabling group mode.
    ////////////////////////////////////////////////////////////////
    vpr::Uint32 written;
    if ( _serial_port != NULL ) {
        char buff[3];

        buff[0] = 'P';
        buff[1] = 0x23;
        buff[2] = 0x01;

        if(_serial_port->write(buff, sizeof(buff), written).success()){
		cout << "success\n";
	}else{
		cout << "failure\n";
	}
	_serial_port->flushQueue(vpr::SerialTypes::IO_QUEUES);
        sleep(2);
    }
}




void aFlock::check_config(){
    if (_serial_port != NULL ){
        char buff[2];
        buff[0] = 'O';
        buff[1] = 0x32;	
	char in[5]; 
        vpr::Uint32 written, buf1;
	
        
        while(true){
	    _serial_port->write(buff, sizeof(buff), written).success();
	    _serial_port->read(in, sizeof(char)*5, buf1);
	    if (buf1>2 && (in[0]==1 || in[0]==0)){
	        std::cout << in << " " << buf1 << std::endl;
	    }
	    std::cout << "sleeping .1" << std::endl;
	    sleep(.1);

	    buf1=0;
	} 
	sleep(2);
    }
	return;
}

void aFlock::check_pos_angles(){
	return;
}

void aFlock::check_rep_and_stream(){
	return;
}

/*
_serial_port->write(exam1, sizeof(char), buf);
	 	_serial_port->write(exam2, sizeof(char), buf);
		_serial_port->readn(in, sizeof(char), buf1);
		std::cout << in[0] << " " << buf << " " << buf1 << std::endl;
*/

void aFlock::disable_hardware_flow_control(){
	_serial_port->disableHardwareFlowControl();
}

void aFlock::enable_hardware_flow_control(){
	_serial_port->enableHardwareFlowControl();
}

