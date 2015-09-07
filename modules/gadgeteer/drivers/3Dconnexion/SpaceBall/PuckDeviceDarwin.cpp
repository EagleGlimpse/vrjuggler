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
//--   PuckDeviceDarwin (source)                                           --
//--   Copyright (c) 2012 Fuel Tech Inc                                    --
//--   Module developed by Daniel Heath (dheath@eagleglimpse.com)          --
//---------------------------------------------------------------------------

// Only Valid for OS X

#include "PuckDeviceDarwin.h"

#include <gadget/Type/DeviceConstructor.h>
#include <gadget/Util/Debug.h>
#include <gadget/Devices/DriverConfig.h>
#include <boost/bind.hpp>
#include <functional>

#include <mach/mach.h>
#include <CoreFoundation/CFNumber.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>

using namespace SpaceBall::Io::Hid;

using std::string;
using boost::bind;
using std::vector;
using std::cout;
using std::cerr;
using std::endl;


extern "C"
{
    // Register this device with the Input Manager
    GADGET_DRIVER_EXPORT(void) initDevice(gadget::InputManager* inputMgr)
    {
        // cout << "[PuckDevice-Darwin] Creating Driver" << endl;
        new gadget::DeviceConstructor<PuckDeviceDarwin>(inputMgr); 
    }

    // Provide entry point for loading shared object
    GADGET_DRIVER_EXPORT(vpr::Uint32) getGadgeteerVersion()
    {
        // cout << "[PuckDevice-Darwin] Getting Gadgeteer Version" << endl;
        return __GADGET_version; 
    }
}

//----------------------------------------------------------------------------
//--  SpaceBall::Io::Hid::DeviceDictionary  ----------------------------------
//----------------------------------------------------------------------------
DeviceDictionary::DeviceDictionary()
    : _ref(0)
{
    _ref = CFDictionaryCreateMutable(
        CFAllocatorGetDefault(),
        0,
        &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks
    );
}

DeviceDictionary::~DeviceDictionary()
{
    if (_ref) CFRelease(_ref);
}

void DeviceDictionary::add(const char* key, int value)
{
    CFStringRef keyR = CFStringCreateWithCString(
        CFAllocatorGetDefault(),
        key,
        CFStringGetSystemEncoding()
    );

    CFNumberRef valueR = CFNumberCreate(
        CFAllocatorGetDefault(), 
        kCFNumberIntType, 
        &value
    );

    CFDictionarySetValue(_ref, keyR, valueR);
    CFRelease(keyR);
    CFRelease(valueR);
}

//----------------------------------------------------------------------------
//--  SpaceBall::Io::Hid::DeviceDictionaryArray  -----------------------------
//----------------------------------------------------------------------------
DeviceDictionaryArray::DeviceDictionaryArray()
    : _ref(0)
{
    _ref = CFArrayCreateMutable(CFAllocatorGetDefault(), 0, &kCFTypeArrayCallBacks);
}

DeviceDictionaryArray::~DeviceDictionaryArray()
{
    if (_ref) CFRelease(_ref);
}

void DeviceDictionaryArray::add(int vendorId, int productId)
{
    DeviceDictionary dict;
    dict.add(kIOHIDVendorIDKey, vendorId);
    dict.add(kIOHIDProductIDKey, productId);

    CFArrayAppendValue(_ref, dict.ref());
}

//----------------------------------------------------------------------------
//--  SpaceBall::Io::Hid::Device  --------------------------------------------
//----------------------------------------------------------------------------
std::string Device::getProperty(const char *name)
{
    const int bufSize = 512;
    char buf[bufSize];
    buf[0] = '\0';
    
    CFStringRef nameR = CFStringCreateWithCString(
        CFAllocatorGetDefault(),
        name,
        CFStringGetSystemEncoding()
    );

    CFTypeRef r = IOHIDDeviceGetProperty(_ref, nameR);
    if (!r) return "<Unknown>";

    if (CFStringGetTypeID() == CFGetTypeID(r)) {
        CFStringGetCString((CFStringRef) r, buf, bufSize, CFStringGetSystemEncoding());
    } else if (CFNumberGetTypeID() == CFGetTypeID(r)) {
        int l;
        bool e = CFNumberGetValue((CFNumberRef) r, kCFNumberSInt32Type, &l);
        if (e) snprintf(buf, bufSize, "0x%x", l);
    } else {
        strcpy(buf, "<Unknown Type>");
    }

    CFRelease(r);

    return std::string(buf);
}

//----------------------------------------------------------------------------
//--  SpaceBall::Io::Hid::Manager  -------------------------------------------
//----------------------------------------------------------------------------
Manager::Manager()
    : _ref(0)
{
    _ref = IOHIDManagerCreate(CFAllocatorGetDefault(), 0);
    IOHIDManagerRegisterDeviceMatchingCallback(_ref, &onMatchingThunk, this);
    IOHIDManagerRegisterDeviceRemovalCallback(_ref, &onRemovalThunk, this);
    IOHIDManagerRegisterInputValueCallback(_ref, &onInputValueThunk, this);
}

Manager::~Manager()
{
    if (_ref) 
    {
        IOHIDManagerRegisterDeviceMatchingCallback(_ref, 0, 0);
        IOHIDManagerRegisterDeviceRemovalCallback(_ref, 0, 0);
        IOHIDManagerRegisterInputValueCallback(_ref, 0, 0);
        // This causes a crash, and I don't know why...
        // CFRelease(_ref);
    }
}

void Manager::setDeviceMatching(int vendorId, int productId)
{
    DeviceDictionary dict;
    dict.add(kIOHIDVendorIDKey, vendorId);
    dict.add(kIOHIDProductIDKey, productId);
    
    IOHIDManagerSetDeviceMatching(_ref, dict.ref());
}

void Manager::setDeviceMatching(DeviceDictionaryArray &list)
{
    IOHIDManagerSetDeviceMatchingMultiple(_ref, list.ref());
}

void Manager::run()
{
    IOHIDManagerScheduleWithRunLoop(_ref, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
    IOHIDManagerOpen(_ref, kIOHIDOptionsTypeNone);
}

void Manager::stop()
{
    IOHIDManagerClose(_ref, kIOHIDOptionsTypeNone);
    IOHIDManagerUnscheduleFromRunLoop(_ref, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
}

void Manager::printMatchingDevice(Device &device)
{
    cout << "[PuckDevice-Darwin] Found Puck Device" << endl;
    cout << "[PuckDevice-Darwin]     Transport:      " << device.getProperty(kIOHIDTransportKey) << endl;
    cout << "[PuckDevice-Darwin]     VendorID:       " << device.getProperty(kIOHIDVendorIDKey) << endl;
    cout << "[PuckDevice-Darwin]     ProductID:      " << device.getProperty(kIOHIDProductIDKey) << endl;
    cout << "[PuckDevice-Darwin]     Manufacturer:   " << device.getProperty(kIOHIDManufacturerKey) << endl;
    cout << "[PuckDevice-Darwin]     Product:        " << device.getProperty(kIOHIDProductKey) << endl;
    cout << "[PuckDevice-Darwin]     SerialNumber:   " << device.getProperty(kIOHIDSerialNumberKey) << endl;
    cout << "[PuckDevice-Darwin]     LocationID:     " << device.getProperty(kIOHIDLocationIDKey) << endl;
    cout << "[PuckDevice-Darwin]     ReportInterval: " << device.getProperty(kIOHIDReportIntervalKey) << endl;
}

void Manager::onMatchingThunk(void *inContext, IOReturn inResult, void * /* inSender */, IOHIDDeviceRef inIOHIDDeviceRef)
{
    Manager *self = (Manager *) inContext;
    Device d(inIOHIDDeviceRef);
    if (inResult == 0) self->onMatchingPrivate(d);
}

void Manager::onRemovalThunk(void *inContext, IOReturn inResult, void * /* inSender */, IOHIDDeviceRef inIOHIDDeviceRef)
{
    Manager *self = (Manager *) inContext;
    Device d(inIOHIDDeviceRef);
    if (inResult == 0) self->onRemovalPrivate(d);
}

void Manager::onInputValueThunk(void *inContext, IOReturn inResult, void * /* inSender */, IOHIDValueRef inIOHIDValueRef)
{
    Manager *self = (Manager *) inContext;
    if (inResult == 0)
    {
        Value v(inIOHIDValueRef);
        self->onInputValuePrivate(v);
    }
}

//----------------------------------------------------------------------------
//--  PuckDeviceDarwin  ------------------------------------------------------
//----------------------------------------------------------------------------
PuckDeviceDarwin::PuckDeviceDarwin()
    : _manager(0)
    , _buttons(15)
    , _axis(6)
{
}

PuckDeviceDarwin::~PuckDeviceDarwin()
{
    if (_manager) stopSampling();
}

bool PuckDeviceDarwin::startSampling()
{
    initBuffers();

    if (!_manager)
    {
        _manager = new Manager();

        DeviceDictionaryArray list;
        _manager->onInputValue(bind(&PuckDeviceDarwin::onInputValue, this, _1));
        _manager->onRemoval(bind(&PuckDeviceDarwin::onRemoval, this, _1));
        list.add(0x046d, 0xC623); // Space Traveler 3
        list.add(0x046d, 0xC625); // Space Pilot
        list.add(0x046d, 0xC626); // Space Navigator
        list.add(0x046d, 0xC627); // Space Explorer
        list.add(0x046d, 0xC628); // Space Navigator for Notebooks
        list.add(0x046d, 0xC629); // Space Pilot PRO
        _manager->setDeviceMatching(list);
        _manager->run();
    } else {
        cout << "[PuckDevice-Darwin] Trying to startSampling when we are already started!!!" << endl;
    }
    return true;
}

bool PuckDeviceDarwin::sample()
{
    return true;
}

bool PuckDeviceDarwin::stopSampling()
{
    _manager->stop();
    delete _manager;
    _manager = 0;
    return true;
}

std::string PuckDeviceDarwin::getElementType()
{
    return string("puck_device");
}

bool PuckDeviceDarwin::config(jccl::ConfigElementPtr c)
{
    if (!gadget::Digital::config(c)) return false;
    if (!gadget::Analog::config(c)) return false;
    if (!gadget::Input::config(c)) return false;

    setMin(-500.0f);
    setMax( 500.0f);

    return true;
}

void PuckDeviceDarwin::updateData()
{
    if (!_manager) return;

    gadget::Analog::swapAnalogBuffers();
    gadget::Digital::swapDigitalBuffers();
}

void PuckDeviceDarwin::onRemoval(SpaceBall::Io::Hid::Device &)
{
    initBuffers();
}

void PuckDeviceDarwin::onInputValue(SpaceBall::Io::Hid::Value &value)
{
    if (value.reportId() == 3)
    {
        int b = value.usage() - 1;
        if (0 <= b && b < 15)
        {
            _buttons[b] = value.asBool();
            gadget::Digital::addDigitalSample(_buttons);            
        }
        cout << "[PuckDevice-Darwin] Button " << value.usage() << " is " << value.asBool() << endl;
    } else {
        switch(value.usage())
        {
        case 48: _axis[0] = normalize( value.asShort()); break;
        case 49: _axis[1] = normalize(-value.asShort()); break;
        case 50: _axis[2] = normalize(-value.asShort()); break;
        case 51: _axis[3] = normalize( value.asShort()); break;
        case 52: _axis[4] = normalize(-value.asShort()); break;
        case 53: _axis[5] = normalize(-value.asShort()); break;
        }
	gadget::Analog::addAnalogSample(_axis);	
    }
}


void PuckDeviceDarwin::initBuffers()
{
    for (std::vector<gadget::AnalogData>::iterator i = _axis.begin() ; i != _axis.end() ; ++i)
	*i = normalize(0.0f);
    gadget::Analog::addAnalogSample(_axis);

    for (std::vector<gadget::DigitalData>::iterator i = _buttons.begin() ; i != _buttons.end() ; ++i)
	*i = 0;
    gadget::Digital::addDigitalSample(_buttons);
}


