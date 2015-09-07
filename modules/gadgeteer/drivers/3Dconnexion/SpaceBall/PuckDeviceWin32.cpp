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
//--   PuckDeviceWin32 (source)                                            --
//--   Copyright (c) 2011 Fuel Tech Inc                                    --
//--   Module developed by Daniel Heath (dheath@eagleglimpse.com)          --
//---------------------------------------------------------------------------

// Only Valid for Windows>
#if defined(WIN32) || defined(WIN64)

#include "PuckDeviceWin32.h"

#include <gadget/Type/DeviceConstructor.h>
#include <gadget/Util/Debug.h>
#include <gadget/Devices/DriverConfig.h>

#include <functional>

using std::string;
using std::tr1::bind;
using std::vector;
using std::cout;
using std::endl;


extern "C"
{
    // Register this device with the Input Manager
    GADGET_DRIVER_EXPORT(void) initDevice(gadget::InputManager* inputMgr)
    { new gadget::DeviceConstructor<PuckDeviceWin32>(inputMgr); }

    // Provide entry point for loading shared object
    GADGET_DRIVER_EXPORT(vpr::Uint32) getGadgeteerVersion()
	{ return __GADGET_version; }
}

PuckDeviceWin32::PuckDeviceWin32()
    : _buttons(15)
    , _axis(6)
    , _running(false)
    , _hWnd(0)
    , _attached(false)
{
}

PuckDeviceWin32::~PuckDeviceWin32()
{
    if (_running) stopSampling();
}

bool PuckDeviceWin32::startSampling()
{
    initBuffers();

    try
    {
	mThread = new vpr::Thread(bind(&PuckDeviceWin32::controlLoop, this));
	_running = true;
    }
    catch (vpr::Exception &ex)
    {
        vprDEBUG(gadgetDBG_INPUT_MGR, vprDBG_CRITICAL_LVL)
	    << clrOutBOLD(clrRED, "ERROR")
	    << ": Failed to spawn thread for 3DConnexion puck_device driver!\n"
	    << vprDEBUG_FLUSH;
        vprDEBUG_NEXT(gadgetDBG_INPUT_MGR, vprDBG_CRITICAL_LVL)
	    << ex.what() << std::endl << vprDEBUG_FLUSH;
        _running = false;
    }
    return _running;
}

bool PuckDeviceWin32::sample()
{
    MSG msg;
    if (GetMessage(&msg, NULL, 0, 0))
    {
	TranslateMessage(&msg);
	DispatchMessage(&msg);
	return true;
    }
    return false;
}

bool PuckDeviceWin32::stopSampling()
{
    _running = false;
    if (mThread != NULL)
    {
	mThread->kill();
	mThread->join();
	delete(mThread);
	mThread = NULL;
    }
    return true;
}

std::string PuckDeviceWin32::getElementType()
{
    return string("puck_device");
}

bool PuckDeviceWin32::config(jccl::ConfigElementPtr c)
{
    if (!gadget::Digital::config(c)) return false;
    if (!gadget::Analog::config(c)) return false;
    if (!gadget::Input::config(c)) return false;

    setMin(-500.0f);
    setMax( 500.0f);

    return true;
}

void PuckDeviceWin32::updateData()
{
    if (!_running) return;

    gadget::Analog::swapAnalogBuffers();
    gadget::Digital::swapDigitalBuffers();
}

void PuckDeviceWin32::controlLoop()
{
    createWindow();
    while (sample());
    CloseWindow(_hWnd);
}

void PuckDeviceWin32::initBuffers()
{
    for (auto i = _axis.begin() ; i != _axis.end() ; ++i)
	*i = normalize(0.0f);
    gadget::Analog::addAnalogSample(_axis);

    for (auto i = _buttons.begin() ; i != _buttons.end() ; ++i)
	*i = 0;
    gadget::Digital::addDigitalSample(_buttons);
}


void PuckDeviceWin32::handleWmCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    registerWithWindows(hWnd);
    _attached = checkForDevice(hWnd);
}

void PuckDeviceWin32::handleWmInputDeviceChange(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    _attached = checkForDevice(hWnd);
    if (!_attached) initBuffers();
}

void PuckDeviceWin32::handleWmInput(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    WPARAM inputCode = GET_RAWINPUT_CODE_WPARAM(wParam);
    HRAWINPUT hRawInput = reinterpret_cast<HRAWINPUT>(lParam);

    // pRawInput = GetRawInputData (hRawInput);
    const size_t cbSizeOfBuffer=1024;
    BYTE pBuffer[cbSizeOfBuffer];

    PRAWINPUT pRawInput = reinterpret_cast<PRAWINPUT>(pBuffer);
    UINT cbSize = cbSizeOfBuffer;

    if (GetRawInputData(hRawInput, RID_INPUT, pRawInput, &cbSize, sizeof(RAWINPUTHEADER)) == NEG_ONE) return;

    processRawInput(inputCode, pRawInput);
    
}

void PuckDeviceWin32::handleWmKillFocus(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
}

void PuckDeviceWin32::createWindow()
{
    HINSTANCE hModule = GetModuleHandle(NULL);

    WNDCLASS wndClass;
    wndClass.style         = 0;
    wndClass.lpfnWndProc   = wndProc;
    wndClass.cbClsExtra    = 0;
    wndClass.cbWndExtra    = 0;
    wndClass.hInstance     = hModule;
    wndClass.hIcon         = NULL;
    wndClass.hCursor       = NULL;
    wndClass.hbrBackground = NULL;
    wndClass.lpszMenuName  = NULL;
    wndClass.lpszClassName = "3Dconnexion Event Window";
    RegisterClass(&wndClass);

    _hWnd = CreateWindow("3Dconnexion Event Window"
	, "3Dconnexion Event Window"
	, WS_OVERLAPPEDWINDOW
	, 100, 100, 100, 100
	, NULL, NULL
	, hModule
	, this
	);

    ShowWindow(_hWnd, SW_HIDE);
    UpdateWindow(_hWnd);
}

LRESULT CALLBACK PuckDeviceWin32::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Set the self pointer.
    PuckDeviceWin32* self;
    if (message == WM_CREATE)
    {
	self = (PuckDeviceWin32*)((LPCREATESTRUCT)lParam)->lpCreateParams;
	SetWindowLongPtr(hWnd,GWLP_USERDATA,(LONG_PTR)self);
    } else {
	self = (PuckDeviceWin32*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
    }
    if (!self) return DefWindowProc(hWnd,message,wParam,lParam);

    switch (message)
    {
    case WM_KILLFOCUS:
	self->handleWmKillFocus(hWnd, wParam, lParam);
	return DefWindowProc(hWnd, message, wParam, lParam);
    case WM_INPUT:
	self->handleWmInput(hWnd, wParam, lParam);
	return DefWindowProc(hWnd, message, wParam, lParam);
    case WM_CREATE:
	self->handleWmCreate(hWnd, wParam, lParam);
	break;
    case WM_INPUT_DEVICE_CHANGE:
	self->handleWmInputDeviceChange(hWnd, wParam, lParam);
	break;
    case WM_DESTROY:
	PostQuitMessage(0);
	break;
    default:
	return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void PuckDeviceWin32::registerWithWindows(HWND hWnd)
{
    _hWnd = hWnd;

    RAWINPUTDEVICE rawInputDevice;
    rawInputDevice.usUsagePage = PUCK_USAGE_PAGE;
    rawInputDevice.usUsage = PUCK_USAGE;
    rawInputDevice.dwFlags = RIDEV_DEVNOTIFY; // Works for Vista and later
    rawInputDevice.dwFlags |= RIDEV_INPUTSINK; // Get all the changes to the device.
    rawInputDevice.hwndTarget = hWnd;

    RegisterRawInputDevices(&rawInputDevice, 1, sizeof(RAWINPUTDEVICE));
}

bool PuckDeviceWin32::checkForDevice(HWND hWnd)
{
    vector<RAWINPUTDEVICELIST> deviceList;
    if (!winGetRawInputDeviceList(deviceList)) return false;

    for (auto i = deviceList.begin() ; i != deviceList.end() ; ++i)
    {
	RID_DEVICE_INFO rdi;
	if (!winGetRawInputDeviceInfo(rdi, i->hDevice)) continue;

	if (rdi.dwType == RIM_TYPEHID &&
	    rdi.hid.dwVendorId == LOGITECH_VENDOR_ID &&
	    rdi.hid.usUsage == PUCK_USAGE &&
	    rdi.hid.usUsagePage == PUCK_USAGE_PAGE)
	{
	    switch (rdi.hid.dwProductId)
	    {
	    case 0xC623: _deviceName = L"SpaceTraveler3"; break;		
	    case 0xC625: _deviceName = L"SpacePilot"; break;
	    case 0xC626: _deviceName = L"SpaceNavigator"; break;
	    case 0xC627: _deviceName = L"SpaceExplorer"; break;
	    case 0xC628: _deviceName = L"SpaceNavigator for Notebooks"; break;
	    case 0xC629: _deviceName = L"SpacePilot PRO"; break;
	    default:
		unsigned int value = rdi.hid.dwProductId;
		wchar_t buf[256];
		swprintf(buf, 255, L"<Unknown - ProductId = 0x%X>", value);
		_deviceName = buf;
		break;
	    };
	    return true;
	}
    }
    return false;
}

bool PuckDeviceWin32::processRawInput(WPARAM inputCode, PRAWINPUT pRawInput)
{
    if (pRawInput->header.dwType != RIM_TYPEHID) return false;

    RID_DEVICE_INFO rdi;
    if (!winGetRawInputDeviceInfo(rdi, pRawInput->header.hDevice)) return false;
    if (rdi.hid.dwVendorId != LOGITECH_VENDOR_ID) return false;

    // NOTES:
    // if (inputCode != RIM_INPUT) then the device is in the background.
    if (pRawInput->data.hid.bRawData[0] == 0x01) // Translation Vector
    {
	short *data = reinterpret_cast<short *>(&pRawInput->data.hid.bRawData[1]);
	_axis[0] = normalize( data[0]);
	_axis[1] = normalize(-data[2]);
	_axis[2] = normalize( data[1]);
	if (pRawInput->data.hid.dwSizeHid >= 13) // Highspeed package
	{
	    _axis[3] = normalize( data[3]);
	    _axis[4] = normalize(-data[5]);
	    _axis[5] = normalize( data[4]);
	}
	gadget::Analog::addAnalogSample(_axis);	
	return true;
    }
    else if (pRawInput->data.hid.bRawData[0] == 0x02) // Rotation Vector
    {
	short *data = reinterpret_cast<short *>(&pRawInput->data.hid.bRawData[1]);
	_axis[3] = normalize( data[0]);
	_axis[4] = normalize(-data[2]);
	_axis[5] = normalize( data[1]);
	gadget::Analog::addAnalogSample(_axis);	
	return true;
    }
    else if (pRawInput->data.hid.bRawData[0] == 0x03) // Key Change
    {
	unsigned short keys = reinterpret_cast<unsigned short *>(&pRawInput->data.hid.bRawData[1])[0];

	for(int i = 0 ; i < 15 ; ++i)
	    _buttons[i] = (0x01 & (keys >> i));
	gadget::Digital::addDigitalSample(_buttons);

//	for (int i = 0 ; i < 15 ; ++i)
//	    cout << _buttons[i].getValue() << " ";
//	cout << endl;

	return true;
    }
    return false;
}

unsigned int PuckDeviceWin32::winGetRawInputDeviceListSize()
{
    unsigned int ret = 0;
    GetRawInputDeviceList(NULL, &ret, sizeof(RAWINPUTDEVICELIST));
    return ret;
}

bool PuckDeviceWin32::winGetRawInputDeviceList(std::vector<RAWINPUTDEVICELIST> &deviceList)
{
    unsigned int deviceCount = winGetRawInputDeviceListSize();
    if (deviceCount == 0) return false;

    deviceList.resize(deviceCount);

    if (GetRawInputDeviceList(&deviceList[0], &deviceCount, sizeof(RAWINPUTDEVICELIST)) == NEG_ONE) return false;
    return true;
}

bool PuckDeviceWin32::winGetRawInputDeviceInfo(RID_DEVICE_INFO &deviceInfo, HANDLE hDevice)
{
    deviceInfo.cbSize = sizeof(RID_DEVICE_INFO);
    unsigned int size = sizeof(RID_DEVICE_INFO);
    UINT ret = GetRawInputDeviceInfo(hDevice, RIDI_DEVICEINFO, &deviceInfo, &size);
    return (ret > 0 && ret != NEG_ONE);
}
#endif