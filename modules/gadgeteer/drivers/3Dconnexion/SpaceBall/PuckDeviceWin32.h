/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2011 by Iowa State University
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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/

//---------------------------------------------------------------------------
//--   PuckDeviceWin32 (header)                                            --
//--   Copyright (c) 2011 Fuel Tech Inc                                    --
//--   Module developed by Daniel Heath (dheath@eagleglimpse.com)          --
//---------------------------------------------------------------------------

#ifndef EagleGlimpse_PuckDeviceWin32_h
#define EagleGlimpse_PuckDeviceWin32_h

// Only Valid for Windows
#if defined(WIN32) || defined(WIN64)

#include <sdkddkver.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinUser.h>

#include <boost/mpl/inherit.hpp>
#include <gadget/gadgetParam.h>
#include <gadget/Type/InputDevice.h>

/// MS Windows Drivers for 3DConnexion Devices
class PuckDeviceWin32
	: public gadget::InputDevice<boost::mpl::inherit<gadget::Digital, gadget::Analog>::type>
{
public:
    /// Constructor
    PuckDeviceWin32();
    /// Destructor
    virtual ~PuckDeviceWin32();

    /** @name Inherited Virtual Functions
    ***
    *** These functions need to be made concrete.
    */
    //@{
    /** Device driver starts sampling.
    *** @return 1 if success, else 0
    */
    virtual bool startSampling();
    /** Function accesses and internally stores device state.
    *** @return 1 if success, else 0
    */
    virtual bool sample();
    /** Device driver stops sampling, kill thread.
    *** @return 1 if success, else 0
    */
    virtual bool stopSampling();
    /** Access function returns configuration chunk type.
    *** @return Name of chunk type
    */
    static std::string getElementType();
    /** Performs necessary configuration to open device; system
    *** passes new vjConfigChunk into this function when a
    *** configuration change is detected for this driver.
    *** @args c Ptr. to a configuration object
    *** @return Success of configuration
    */
    virtual bool config(jccl::ConfigElementPtr c);
    /** Swaps data and vjInput:: indexes in triple-buffered data
     */
    virtual void updateData();
    //@}
private:
    ///
    void controlLoop();
    /// Initialize the analog and digital buffers
    void initBuffers();
    /// Digital data to be sent to gadget classes
    std::vector<gadget::DigitalData> _buttons;
    /// Analog data to be sent to gadget classes
    std::vector<gadget::AnalogData> _axis;
    ///
    int _axesCount[6];
    ///
    bool _running;
    ///
    int _updateCount;
    
    /** @name WndProc callback Functions.
    ***
    *** These functions should be called from the main WndProc callback.
    */
    //@{
    /// Handles WM_CREATE message
    void handleWmCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
    /// Handles WM_INPUT_DEVICE_CHANGE message
    void handleWmInputDeviceChange(HWND hWnd, WPARAM wParam, LPARAM lParam);
    /// Handles WM_INPUT message
    void handleWmInput(HWND hWnd, WPARAM wParam, LPARAM lParam);
    /// Handles WM_KILLFOCUS message
    void handleWmKillFocus(HWND hWnd, WPARAM wParam, LPARAM lParam);
    //@}
    /// Create a window to get Puck Events
    void createWindow();
    /// Windows callback Funcion.
    static LRESULT CALLBACK wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /// Register to recieve required call back function.
    void registerWithWindows(HWND hWnd);
    /// Returns true if the device is available.
    bool checkForDevice(HWND hWnd);

    bool processRawInput(WPARAM inputCode, PRAWINPUT pRawInput);

    /// Window Handle that is handling the messages.
    HWND _hWnd;
    /// True if a device is attached.
    bool _attached;
    /// The name of the device that is connected.
    std::wstring _deviceName;
	
    /// Logitech Vendor Id
    static const DWORD LOGITECH_VENDOR_ID = 0x46d;
    /// Usage Page used to identify the device.
    static const USHORT PUCK_USAGE_PAGE = 0x01;
    /// Usage used to identify the device.
    static const USHORT PUCK_USAGE = 0x08;

    /** @name Repairing Badly formed windows Functions.
    ***
    *** These are direct calls to windows functions, but they are better
    *** formed.
    */
    //@{
    static const UINT NEG_ONE = static_cast<UINT>(-1);

    unsigned int winGetRawInputDeviceListSize();
    bool winGetRawInputDeviceList(std::vector<RAWINPUTDEVICELIST> &deviceList);
    bool winGetRawInputDeviceInfo(RID_DEVICE_INFO &deviceInfo, HANDLE hDevice);

    //@}
};

#endif
#endif