// Copied from public information at: http://www.evl.uic.edu/pape/CAVE/prog/
//
// Thanks Dave.
//
#ifndef _TRACKD_MEM_H_
#define _TRACKD_MEM_H_

#include <vjDefines.h>
#include <sys/types.h>

#if defined(VJ_OS_Linux)
#   include <stdint.h>
#endif

struct CAVE_SENSOR_ST
   {
   float       x, y, z;
   float       azim, elev, roll;
   uint32_t    timestamp[2];
   int32_t     calibrated;
   int32_t     frame;
   };

struct CAVE_TRACKDTRACKER_HEADER
{
   uint32_t version;       /*************** <auto-copyright.pl BEGIN do not edit this line> **************
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
   uint32_t numSensors;    /* Total number of sensors */
   uint32_t sensorOffset;  /* Byte offset from of header to start of sensor array */
   uint32_t sensorSize;    /* sizeof() of a sensor struct */
   uint32_t timestamp[2];  /* NB: *Not* a struct timeval - that changes */
                           /* size between 32 & 64 bit mode */
   uint32_t command;       /* For sending commands (such as 'reset') to daemon */
} ;

struct CAVE_TRACKDCONTROLLER_HEADER
{
   uint32_t version;          /* CAVElib version (see constants below) */
   uint32_t buttonOffset;     /* Byte offset from start of header to start of button data */
   uint32_t valuatorOffset;   /* Byte offset from start of header to start of valuator data */
   uint32_t numButtons;
   uint32_t numValuators;
   uint32_t timestamp[2];
   uint32_t command;          /* For sending commands (such as 'reset') to daemon */
};

/* Command values for header.command */
/*   header.command should be 0 when no command is pending */
#define CAVE_TRACKD_NO_COMMAND      0
#define CAVE_TRACKD_RESET_COMMAND      1

/* ID Numbers for CAVElib version */
/*  Increasing values should be assigned for newer library versions whenever
    either the header, sensor, or controller struct definition is expanded */
#define CAVELIB_2_6  1


void * trackd_attach_tracker_mem(int shmKey);
void trackd_release_tracker_mem(void *mem);
int trackd_num_sensors(void *mem);
CAVE_SENSOR_ST * trackd_sensor(void *mem,int sensorNum);
void * trackd_attach_controller_mem(int shmKey);
void trackd_release_controller_mem(void *mem);
int trackd_num_buttons(void *mem);
int trackd_num_valuators(void *mem);
int trackd_button(void *mem,int buttonNum);
float trackd_valuator(void *mem,int valuatorNum);

#endif