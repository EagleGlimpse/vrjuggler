/****************** <VPR heading BEGIN do not edit this line> *****************
 *
 * VR Juggler Portable Runtime
 *
 * Original Authors:
 *   Allen Bierbaum, Patrick Hartling, Kevin Meinert, Carolina Cruz-Neira
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 *
 ****************** <VPR heading END do not edit this line> ******************/

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

#ifndef _VPR_SERIAL_PORT_IMPL_WIN32_H_
#define _VPR_SERIAL_PORT_IMPL_WIN32_H_

#include <vpr/vprConfig.h>

#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

#include <vpr/IO/IOSys.h>
#include <vpr/IO/Port/SerialTypes.h>
#include <vpr/Util/ReturnStatus.h>
#include <vpr/Util/Interval.h>
#include <vpr/Util/Debug.h>


namespace vpr
{

/**
 * vpr::SerialPort implementation for Win32.
 */
class VPR_CLASS_API SerialPortImplWin32
{
public:
   /**
    * Constructor.  This creates a file handle object connected to the given
    * port name and sets the update action to happen immediately.
    *
    * @post mHandle is assigned a value and connected to port_name, and the
    *       update action is set to vpr::SerialTypes::NOW.
    *
    * @param port_name The name of the serial port that will be accessed.
    */
   SerialPortImplWin32(const std::string& port_name);

   /**
    * Destructor.  If the serial port is still open, it is closed.
    *
    * @post The serial port is closed.
    */
   ~SerialPortImplWin32(void);

   // ========================================================================
   // vpr::BlockIO basics.
   // ========================================================================

   /**
    * Gets the name of this serial port.
    *
    * @post
    *
    * @return An object containing the name of the serial port.
    */
   const std::string& getName (void)
   {
      return mName;
   }

   /**
    * Sets the open flags so that the serial port is opened in read-only
    * mode.
    *
    * @post The open flags are updated so that when the port is opened, it is
    *       opened in read-only mode.  If the port is already open, this has
    *       no effect.
    */
   void setOpenReadOnly (void)
   {
      mOpenFlag = GENERIC_READ;
   }

   /**
    * Sets the open flags so that the serial port is opened in write-only
    * mode.
    *
    * @post The open flags are updated so that when the port is opened, it is
    *       opened in write-only mode.  If the port is already open, this has
    *       no effect.
    */
   void setOpenWriteOnly (void)
   {
      mOpenFlag = GENERIC_WRITE;
   }

   /**
    * Sets the open flags so that the serial port is opened in read/write
    * mode.
    *
    * @post The open flags are updated so that when the port is opened, it is
    *       opened in read/write mode.  If the port is already open, this has
    *       no effect.
    */
   void setOpenReadWrite (void)
   {
      mOpenFlag = GENERIC_READ | GENERIC_WRITE;
   }

   /**
    * Sets the blocking flags so that the serial port is opened in blocking
    * mode.
    *
    * @post The open flags are updated so that when the port is opened, it
    *       is opened in blocking mode.  If the port is already open, this
    *       has no effect.
    */
   void setOpenBlocking (void)
   {
      std::cout << "'setOpenBlocking' not implemented for Win32" << std::endl;
   }

   /**
    * Sets the blocking flags so that the serial port is opened in
    * non-blocking mode.
    *
    * @post The open flags are updated so that when the port is opened, it
    *       is opened in non-blocking mode.  If the port is already open,
    *       this has no effect.
    */
   void setOpenNonBlocking (void)
   {
      std::cout << "'setOpenBlocking' not implemented for Win32" << std::endl;
   }

   /**
    * Opens the serial port and initializes its flags.
    *
    * @pre The serial port is not already open.
    * @post An attempt is made to open the port.  If it is successful, the
    *       port's flags are initaialized to 0.  The resulting status is
    *       returned to the caller.  If the port is opened, mOpen is set to
    *       true.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the serial port was opened
    *         successfully.<br>
    *         vpr::ReturnStatus::Fail is returned otherwise.
    */
   vpr::ReturnStatus open(void);

   /**
    * Closes the serial port.
    *
    * @pre The serial port is open.
    * @post An attempt is made to close the port.  The resulting status is
    *       returned to the caller.  If the port is closed, mOpen is set to
    *       false.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the serial port was closed
    *         successfully.<br>
    *         vpr::ReturnStatus::Fail is returned otherwise.
    */
   vpr::ReturnStatus close (void)
   {
      vpr::ReturnStatus retval;

      if ( !CloseHandle(mHandle) )
      {
         retval.setCode(vpr::ReturnStatus::Fail);
      }
      return retval;
   }

   /**
    * Reconfigures the serial port so that it is in blocking mode.
    *
    * @pre The serial port is open.
    * @post Processes will block when accessing the port.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the blocking mode was
    *         changed successfully; vpr::ReturnStatus::Fail otherwise.
    */
   vpr::ReturnStatus enableBlocking (void)
   {
      vpr::ReturnStatus status(vpr::ReturnStatus::Fail);
      std::cout << "Enabling blocking mode after port open is unsuported in Win32." << std::endl;
      return status;
   }

   /**
    * Reconfigures the serial port so that it is in non-blocking mode.
    *
    * @pre The serial port is open.
    * @post Processes will not block when accessing the port.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the blocking mode was
    *         changed successfully; vpr::ReturnStatus::Fail otherwise.
    */
   vpr::ReturnStatus enableNonBlocking (void)
   {
      vpr::ReturnStatus status(vpr::ReturnStatus::Fail);
      std::cout << "Enabling Nonblocking mode after port open is unsuported in Win32." << std::endl;
      return status;
   }

   /**
    * Get the current blocking state for the serial port.
    *
    * @pre mBlocking is set correctly
    *
    * @return true is returned if the port is in blocking mode.<br>
    *         false is returned if the port is in non-blocking mode.
    */
   bool getBlocking (void) const
   {
      return mBlocking;
   }

   /**
    * Gets the current non-blocking state for the serial port.
    *
    * @pre <code>mBlocking</code> is set correctly<br>
    *
    * @return <code>true</code> is returned if the port is in non-blocking
    *         mode.   Otherwise, <code>false</code> is returned.
    */
   bool getNonBlocking (void) const
   {
      return ! mBlocking;
   }

   /**
    * Returns the contained handle.
    */
   vpr::IOSys::Handle getHandle (void)
   {
#ifdef VPR_USE_NSPR
      vprDEBUG(vprDBG_ALL, vprDBG_CRITICAL_LVL)
         << "ERROR: Cannot get handle for Win32 file descriptor with NSPR!\n";
      return vpr::IOSys::NullHandle;
#else
      return mHandle;
#endif
   }

   /**
    * Tests if the serial port is read-only.
    *
    * @pre The serial port is open.
    * @post The access mode is tested for read-only mode, and the result is
    *       returned to the caller.
    *
    * @return <code>true</code> is returned if the device is in read-only
    *         mode; <code>false</code> otherwise.
    */
   bool isReadOnly (void)
   {
      return GENERIC_READ == mOpenFlag;
   }

   /**
    * Tests if the serial port is write-only.
    *
    * @pre The serial port is open.
    * @post The access mode is tested for write-only mode, and the result is
    *       returned to the caller.
    *
    * @return <code>true</code> is returned if the device is in write-only
    *         mode; <code>false</code> otherwise.
    */
   bool isWriteOnly (void)
   {
      return GENERIC_WRITE == mOpenFlag;
   }

   /**
    * Tests if the serial port is read/write.
    *
    * @pre The serial port is open.
    * @post The access mode is tested for read/write mode, and the result is
    *       returned to the caller.
    *
    * @return <code>true</code> is returned if the device is in read/write
    *         mode; <code>false</code> otherwise.
    */
   bool isReadWrite (void)
   {
      return(GENERIC_READ | GENERIC_WRITE) == mOpenFlag;
   }

   // ========================================================================
   // VPR serial port interface implementation.
   // ========================================================================

   /**
    * Gets the current update action.  This tells when updates to the serial
    * device take effect.
    *
    * @post The current update action is returned to the caller.
    *
    * RETURNS: A vpr::SerialTypes::UpdateActionOption value stating when
    *          updates take effect.
    */
   vpr::SerialTypes::UpdateActionOption getUpdateAction(void);

   /**
    * Changes the current update action to take place as described by the
    * given value.
    *
    * @post The update action is modified to use the new value.  The results
    *       will be seen with the next device operation.
    *
    * @param action The new update action value.
    */
   void setUpdateAction (vpr::SerialTypes::UpdateActionOption action);

   /**
    * Queries the serial port for the maximum buffer size.
    *
    * @pre The serial port is open.
    * @post The maximum buffer size is returned to the caller through the
    *       by-reference argument.
    *
    * @param size A reference to a vpr::Uint8 where the buffer size is
    *             stored for return to the caller.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the buffer size was
    *         retrieved successfully; vpr::ReturnStatus::Fail otherwise.
    */
   vpr::ReturnStatus getBufferSize(vpr::Uint16& size);

   /**
    * Attempts to change the buffer size to the given argument.
    *
    * @pre The serial port is open.
    * @post If the buffer size is usable on the port, the port attributes
    *       are updated and success is returned.  Otherwise, a failure
    *       status is returned.
    *
    * @param size The new size for the buffer.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the buffer size was set
    *         successfully; vpr::ReturnStatus::Fail otherwise.
    */
   vpr::ReturnStatus setBufferSize(const vpr::Uint8 size);

   /**
    * Gets the value of the timeout (in tenths of a second) to wait for data
    * to arrive.  This is only applicable in non-canonical mode.
    *
    * @pre The serial port is open.
    * @post The current timeout setting is returned to the caller in the
    *       by-reference argument.
    *
    * @param timeout A reference to a vpr::Uint8 to be used as storage for
    *                the timeout value.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the timeout was requested
    *         successfully; vpr::ReturnStatus::Fail otherwise.
    */
   vpr::ReturnStatus getTimeout(vpr::Uint8& timeout);

   /**
    * Sets the value of the timeout to wait for data to arrive.  The value
    * given must be in tenths of a second.  This is only applicable in
    * non-canonical mode.
    *
    * @pre The serial port is open.
    * @post The timeout interval is updated to use the given value.
    *
    * @param timeout_val The new timeout value measured in tenths of a
    *                    second.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the timeout was set
    *         successfully; vpr::ReturnStatus::Fail otherwise.
    */
   vpr::ReturnStatus setTimeout(const vpr::Uint8 timeout);

   // ------------------------------------------------------------------------
   //: Get the character size (the bits per byte).
   //
   //! PRE: The serial port is open.
   //! POST: The current character size is returned to the caller in the
   //+       by-reference argument.
   //
   //! ARGS: size - A reference to a vpr::SerialTypes::CharacterSizeOption
   //+              object to be used as storage for the character size
   //+              value.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus getCharacterSize(vpr::SerialTypes::CharacterSizeOption& size);

   // ------------------------------------------------------------------------
   //: Set the current character size (the bits per byte) to the size in the
   //+ given value.  This is used for both reding and writing, and the size
   //+ does not include the parity bit (if any).
   //
   //! PRE: The serial port is open.
   //! POST: The bits per byte for the serial port is updated.
   //
   //! ARGS: bpb - The new character size.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus setCharacterSize(const vpr::SerialTypes::CharacterSizeOption bpb);

   /**
    * Gets the current read state for the port.
    *
    * @pre The serial port is open.
    * @post The read state (either enabled or disabled) is returne to the
    *       caller.
    *
    * @return <code>true</code> is returned if bytes can be read from the
    *         device.
    */
   bool getReadState (void)
   {
      return true;
   }

   /**
    * Enables the receiver so that bytes can be read from the port.
    *
    * @pre The serial port is open.
    * @post An attempt is made to make the device readable.
    *
    * @return vpr::ReturnStatus::Succeed is returned if reading is enabled
    *         successfully.
    */
   vpr::ReturnStatus enableRead (void)
   {
      return vpr::ReturnStatus();
   }

   /**
    * Disables the receiver so that bytes cannot be read from the port.
    *
    * @pre The serial port is open.
    * @post An attempt is made to make the device unreadable.
    *
    * @return A vpr::ReturnStatus object describing the results of the operation.
    */
   vpr::ReturnStatus disableRead (void)
   {
      return vpr::ReturnStatus(vpr::ReturnStatus::Fail);
   }

   // ------------------------------------------------------------------------
   //: Get the number of stop bits in use.  This will be either 1 or 2.
   //
   //! PRE: The serial port is open.
   //! POST: The number of stop bits currently in use is returned via the
   //+       by-reference argument.
   //
   //! ARGS: num_bits - A reference to a vpr::Uint8 that will be used to
   //+                  store the number of stop bits.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus getStopBits(vpr::Uint8& num_bits);

   // ------------------------------------------------------------------------
   //: Set the number of stop bits to use.  The value must be either 1 or 2.
   //
   //! PRE: The serial port is open, and the given number of stop bits is
   //+      either 1 or 2.
   //! POST: The number of stop bits is updated to use the given value.
   //
   //! ARGS: num_bits - The number of stop bits to use.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus setStopBits(const vpr::Uint8 num_bits);

   // ------------------------------------------------------------------------
   //: Query the canonical input state of the serial port.  If canonical mode
   //+ is enabled, the characters EOF, EOL, EOL2, ERASE, KILL, REPRINT,
   //+ STATUS, and WERASE are enabled, and the input characters are
   //+ assembled into lines.  Otherwise, read requests are satisfied directly
   //+ from the input queue, and a read will not return until the buffer is
   //+ at its minimum capacity or the timeout has expired.  See
   //+ getBufferSize() and getTimeout() for more information.
   //
   //! PRE: The serial port is open.
   //! POST: The current state of canonical input is returnd to the caller.
   //
   //! RETURNS: true  - Canonical input is enabled.
   //! RETURNS: false - Canonical input is disabled.
   // ------------------------------------------------------------------------
   bool getCanonicalState(void);

   // ------------------------------------------------------------------------
   //: Enable canonical input.  See getCanonicalState() for more information
   //+ about what this means.
   //
   //! PRE: The serial port is open.
   //! POST: Canonical input is enabled.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus enableCanonicalInput(void);

   // ------------------------------------------------------------------------
   //: Disable canonical input.  See getCanonicalState() for more information
   //+ about what this means.
   //
   //! PRE: The serial port is open.
   //! POST: Canonical input is disabled.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus disableCanonicalInput(void);

   // ------------------------------------------------------------------------
   //: Get the current state of ignoring bytes with framing errors (other
   //+ than a BREAK) or parity errors.
   //
   //! PRE: The serial port is open.
   //! POST: The erroneous byte ignore state is returned to the caller.
   //
   //! RETURNS: true  - Bad bytes are ignored.
   //! RETURNS: false - Bad bytes are not ignored.
   // ------------------------------------------------------------------------
   bool getBadByteIgnoreState(void);

   // ------------------------------------------------------------------------
   //: Enable ignoring of received bytes with framing errors or parity
   //+ errors.
   //
   //! PRE: The serial port is open.
   //! POST: Erroneous byte ignoring is enabled.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus enableBadByteIgnore(void);

   // ------------------------------------------------------------------------
   //: Disable ignoring of received bytes with framing errors or parity
   //+ errors.
   //
   //! PRE: The serial port is open.
   //! POST: Erroneous byte ignoring is disabled.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus disableBadByteIgnore(void);

   // ------------------------------------------------------------------------
   //: Get the state of parity checking for input.
   //
   //! PRE: The serial port is open.
   //! POST: The state of input parity checking is returned to the caller.
   //
   //! RETURNS: true  - Input parity checking is enabled.
   //! RETURNS: false - Input parity checking is disabled.
   // ------------------------------------------------------------------------
   bool getInputParityCheckState(void);

   // ------------------------------------------------------------------------
   //: Enable input parity checking.
   //
   //! PRE: The serial port is open.
   //! POST: Input parity checking is enabled.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus enableInputParityCheck(void);

   // ------------------------------------------------------------------------
   //: Disable input parity checking.
   //
   //! PRE: The serial port is open.
   //! POST: Input parity checking is disabled.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus disableInputParityCheck(void);

   // ------------------------------------------------------------------------
   //: Get the current state of bit stripping.  When enabled, input bytes are
   //+ stripped to seven bits.  Otherwise, all eight bits are processed.
   //
   //! PRE: The serial port is open.
   //! POST: The current bit stripping state is returned to the caller.
   //
   //! RETURNS: true  - Input bytes are stripped to seven bits.
   //! RETURNS: false - Input bytes are not stripped.
   // ------------------------------------------------------------------------
   bool getBitStripState(void);

   // ------------------------------------------------------------------------
   //: Enable stripping of input bytes to seven bits.
   //
   //! PRE: The serial port is open.
   //! POST: Input bit stripping is enabled.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus enableBitStripping(void);

   // ------------------------------------------------------------------------
   //: Disable stripping of input bytes to seven bits.
   //
   //! PRE: The serial port is open.
   //! POST: Input bit stripping is disabled.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus disableBitStripping(void);

   // ------------------------------------------------------------------------
   //: Get the state of start-stop input control.  When enabled, if the
   //+ terminal driver sees that the receive buffer is getting full, a STOP
   //+ command is sent to the serial device.  The device should respond by
   //+ halting its data flow.  When the input queue has been processed, a
   //+ START command is sent to the serial device which should cause it to
   //+ resume putting bytes onto the input queue.
   //
   //! PRE: The serial port is open.
   //! POST: The current state of start-stop input control is returned to the
   //+       caller.
   //
   //! RETURNS: true  - Start-stop input control is enabled.
   //! RETURNS: true  - Start-stop input control is not enabled.
   // ------------------------------------------------------------------------
   bool getStartStopInputState(void);

   // ------------------------------------------------------------------------
   //: Enable start-stop input control.  See getStartStopInputState() for
   //+ more information.
   //
   //! PRE: The serial port is open.
   //! POST: Start-stop input control is enabled.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus enableStartStopInput(void);

   // ------------------------------------------------------------------------
   //: Disable start-stop input control.  See getStartStopInputState() for
   //+ more information.
   //
   //! PRE: The serial port is open.
   //! POST: Start-stop input control is disabled.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus disableStartStopInput(void);

   // ------------------------------------------------------------------------
   //: Get the state of start-stop output control.  When enabled, when the
   //+ terminal driver receives a STOP command, output stops.  When it
   //+ receives a START command, output resumes.  If disabled, these
   //+ commands are read as normal characters.
   //
   //! PRE: The serial port is open.
   //! POST: The current state of start-stop output control is returned to
   //+       the caller.
   //
   //! RETURNS: true  - Start-stop output control is enabled.
   //! RETURNS: true  - Start-stop output control is not enabled.
   // ------------------------------------------------------------------------
   bool getStartStopOutputState(void);

   // ------------------------------------------------------------------------
   //: Enable start-stop input control.  See getStartStopInputState() for
   //+ more information.
   //
   //! PRE: The serial port is open.
   //! POST: Start-stop input control is ensabled.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus enableStartStopOutput(void);

   // ------------------------------------------------------------------------
   //: Disable start-stop output control.  See getStartStopOutputState() for
   //+ more information.
   //
   //! PRE: The serial port is open.
   //! POST: Start-stop output control is disabled.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus disableStartStopOutput(void);

   // ------------------------------------------------------------------------
   //: Get the current state of parity generation for outgoing bytes and
   //+ parity checking for incoming bytes.
   //
   //! PRE: The serial port is open.
   //! POST: The current parity generation/checking state is returned to the
   //+       caller.
   //
   //! RETURNS: true  - Parity generation and checking are enabled for
   //+                  outgoing and incoming bytes respectively.
   //! RETURNS: false - Parity generation and checking are disabled.
   // ------------------------------------------------------------------------
   bool getParityGenerationState(void);

   // ------------------------------------------------------------------------
   //: Enable parity generation for outgoing bytes and parity checking for
   //+ incoming bytes.
   //
   //! PRE: The serial port is open.
   //! POST: Parity generation and checking are enabled.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus enableParityGeneration(void);

   // ------------------------------------------------------------------------
   //: Disable parity generation for outgoing bytes and parity checking for
   //+ incoming bytes.
   //
   //! PRE: The serial port is open.
   //! POST: Parity generation and checking are disabled.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus disableParityGeneration(void);


   // ------------------------------------------------------------------------
   //: Enable marking of bytes with parity errors or framing errors (except
   //+ BREAKs).  This is only active if input parity and framing error
   //+ reporting is enabled (see getInputParityCheckState() for more
   //+ information).  The mark is the three-byte sequence \377 \0 X where X
   //+ is the byte received in error.  If bit stripping is enabled, a valid
   //+ \377 byte is passed as the two-byte sequence \377 \377.
   //
   //! PRE: The serial port is open.
   //! POST: Bytes with an error are marked and passed on to the reader.
   //
   //! RETURNS: true  - Parity error marking is enabled.
   //! RETURNS: false - Parity error marking is disabled.
   // ------------------------------------------------------------------------
   bool getParityErrorMarkingState(void);

   // ------------------------------------------------------------------------
   //: Enable parity error and framing error marking.
   //
   //! PRE: The serial port is open.
   //! POST: Parity error marking is enabled.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus enableParityErrorMarking(void);

   // ------------------------------------------------------------------------
   //: Disable parity error and framing error marking.
   //
   //! PRE: The serial port is open.
   //! POST: Parity error marking is disabled.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus disableParityErrorMarking(void);

   // ------------------------------------------------------------------------
   //: Get the current parity checking type (either odd or even).
   //
   //! PRE: The serial port is open.
   //! POST: The parity type is returned to the caller.
   //
   //! RETURNS: A vpr::SerialTypes::ParityType value giving the parity
   //+          checking being done.
   // ------------------------------------------------------------------------
   SerialTypes::ParityType getParity(void);

   // ------------------------------------------------------------------------
   //: Enable odd parity.
   //
   //! PRE: The serial port is open.
   //! POST: Odd parity is enabled.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus setOddParity(void);

   // ------------------------------------------------------------------------
   //: Enable even parity.
   //
   //! PRE: The serial port is open.
   //! POST: Even parity is enabled.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus setEvenParity(void);

   // ------------------------------------------------------------------------
   //: Get the current input baud rate.
   //
   //! PRE: The serial port is open.
   //! POST: The current input baud rate is returned to the caller via the
   //+       by-reference parameter.
   //
   //! ARGS: rate - A reference to a vpr::Uint32 to be used as storage for
   //+              the current input baud rate.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus getInputBaudRate(vpr::Uint32& rate);

   // ------------------------------------------------------------------------
   //: Set the current input baud rate.
   //
   //! PRE: The serial port is open, and the rate is a valid baud rate.
   //! POST: The input baud rate is updated to use the given value.
   //
   //! ARGS: rate - The new input baud rate.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus setInputBaudRate(const vpr::Uint32 rate);

   // ------------------------------------------------------------------------
   //: Get the current output baud rate.
   //
   //! PRE: The serial port is open.
   //! POST: The current output baud rate is returned to the caller via the
   //+       by-reference parameter.
   //
   //! ARGS: rate - A reference to a vpr::Uint32 to be used as storage for
   //+              the current output baud rate.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus getOutputBaudRate(vpr::Uint32& rate);

   // ------------------------------------------------------------------------
   //: Set the current output baud rate.
   //
   //! PRE: The serial port is open, and the rate is a valid baud rate.
   //! POST: The output baud rate is updated to use the given value.
   //
   //! ARGS: rate - The new output baud rate.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus setOutputBaudRate(const vpr::Uint32 rate);



   // ------------------------------------------------------------------------
   //: Wait for all output to be transmitted.
   //
   //! PRE: The serial port is open.
   //! POST: All output is transmitted.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus drainOutput(void);

   // ------------------------------------------------------------------------
   //: Alter the input or output flow control.  Based on the
   //+ vpr::SerialTypes::FlowActionOption argument, output can be suspended
   //+ and restarted or the terminal device can be told to stop or to resume
   //+ sending data.
   //
   //! PRE: The serial device is open.
   //! POST: Based on the argument value, the input or output flow control is
   //+       updated.
   //
   //! ARGS: opt - The flow control change.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus controlFlow(vpr::SerialTypes::FlowActionOption opt);

   // ------------------------------------------------------------------------
   //: Discard either the input buffer (unread data received from the
   //+ terminal device) or the output buffer (data written but not yet
   //+ transmitted to the terminal device).  The argument tells which queue
   //+ (or queues) to flush.
   //
   //! PRE: The serial device is open.
   //! POST: Based on the argument, the appropriate queue is flushed of its
   //+       current contents.
   //
   //! ARGS: queue - The queue (or queues) to be flushed.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus flushQueue(vpr::SerialTypes::FlushQueueOption queue);




   // ------------------------------------------------------------------------
   //: Transmit a continuous stream of zero bits for the given duration.  If
   //+ the argument is 0, the transmission will last between 0.25 and 0.5
   //+ seconds.  Otherwise, the duration specfies the number of seconds to
   //+ send the zero bit stream.
   //
   //! PRE: The serial device is open.
   //! POST: A stream of zero bits are sent to the serial device for the
   //+       given duration.
   //
   //! ARGS: duration - The length of time to send a zero bit stream to the
   //+                  serial device.
   //
   //! RETURNS: A vpr::ReturnStatus object describing the results of the operation.
   // ------------------------------------------------------------------------
   vpr::ReturnStatus sendBreak(const vpr::Int32 duration);

   /**
    * Implementation of the <code>read</code> template method.  This reads at
    * most the specified number of bytes from the serial port into the given
    * buffer.
    *
    * @pre The port is open for reading, and the buffer is at least
    *      <code>length</code> bytes long.
    * @post The given buffer has length bytes copied into it from the port,
    *       and the number of bytes read successfully is returned to the
    *       caller via the <code>bytes_read</code> parameter.
    *
    * @param buffer     A pointer to the buffer where the port's buffer
    *                   contents are to be stored.
    * @param length     The number of bytes to be read.
    * @param bytes_read The number of bytes read into the buffer.
    * @param timeout    The maximum amount of time to wait for data to be
    *                   available for reading.  This argument is optional and
    *                   defaults to vpr::Interval::NoTimeout.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the read operation
    *         completed successfully.<br>
    *         vpr::ReturnStatus::WouldBlock if the port is in non-blocking mode,
    *         and there is no data to read.<br>
    *         vpr::ReturnStatus::Timeout is returned if the read could not begin
    *         within the timeout interval.<br>
    *         vpr::ReturnStatus::Fail is returned if the read operation failed.
    */
   vpr::ReturnStatus read_i (void* buffer, const vpr::Uint32 length,
                             vpr::Uint32& bytes_read,
                             const vpr::Interval timeout = vpr::Interval::NoTimeout);

   /**
    * Implementation of the <code>readn</code> template method.  This reads
    * exactly the specified number of bytes from the serial port into the
    * given buffer.
    *
    * @pre The port is open for reading, and the buffer is at least
    *      <code>length</code> bytes long.
    * @post The given buffer has <code>length</code> bytes copied into
    *       it from the port, and the number of bytes read successfully
    *       is returned to the caller via the <code>bytes_read</code>
    *       parameter.
    *
    * @param buffer     A pointer to the buffer where the ports's buffer
    *                   contents are to be stored.
    * @param length     The number of bytes to be read.
    * @param bytes_read The number of bytes read into the buffer.
    * @param timeout    The maximum amount of time to wait for data to be
    *                   available for reading.  This argument is optional and
    *                   defaults to vpr::Interval::NoTimeout.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the read operation
    *         completed successfully.<br>
    *         vpr::ReturnStatus::WouldBlock if the port is in non-blocking mode,
    *         and there is no data to read.<br>
    *         vpr::ReturnStatus::Timeout is returned if the read could not begin
    *         within the timeout interval.<br>
    *         vpr::ReturnStatus::Fail is returned if the read operation failed.
    */
   vpr::ReturnStatus readn_i (void* buffer, const vpr::Uint32 length,
                              vpr::Uint32& bytes_read,
                              const vpr::Interval timeout = vpr::Interval::NoTimeout)
   {
      // XXX: Fix me!!!
      return vpr::ReturnStatus();
   }

   /**
    * Implementation of the <code>write</code> template method.  This writes
    * the given buffer to the serial port.
    *
    * @pre The port is open for writing.
    * @post The given buffer is written to the I/O port, and the number
    *       of bytes written successfully is returned to the caller via the
    *       <code>bytes_written</code> parameter.
    *
    * @param buffer        A pointer to the buffer to be written.
    * @param length        The length of the buffer.
    * @param bytes_written The number of bytes written to the port.
    * @param timeout       The maximum amount of time to wait for data to be
    *                      available for writing.  This argument is optional
    *                      and defaults to vpr::Interval::NoTimeout.
    *
    * @return vpr::ReturnStatus::Succeed is returned if the write operation
    *         completed successfully.<br>
    *         vpr::ReturnStatus::WouldBlock is returned if the handle is in
    *         non-blocking mode, and the write operation could not be
    *         completed.<br>
    *         vpr::ReturnStatus::Timeout is returned if the write could not begin
    *         within the timeout interval.<br>
    *         vpr::ReturnStatus::Fail is returned if the write operation failed.
    */
   vpr::ReturnStatus write_i (const void* buffer, const vpr::Uint32 length,
                              vpr::Uint32& bytes_written,
                              const vpr::Interval timeout = vpr::Interval::NoTimeout);

protected:
   HANDLE mHandle;    /**< handle to communication file */
   DWORD mOpenFlag;     /**< flag to specify how to open comm port */
   std::string mName; /**< name of communication device */
   bool mOpen;
   bool mBlocking;    /**< flag for blocking I/O */
   bool mParityMark;    /**< flag for parity marking */
};

} // End of vpr namespace


#endif  /* _VPR_SERIAL_PORT_IMPL_WIN32_H_ */
