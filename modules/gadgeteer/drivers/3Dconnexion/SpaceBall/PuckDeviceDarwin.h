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
//--   PuckDeviceDarwin (header)                                           --
//--   Copyright (c) 2012 Fuel Tech Inc                                    --
//--   Module developed by Daniel Heath (dheath@eagleglimpse.com)          --
//---------------------------------------------------------------------------

#ifndef EagleGlimpse_PuckDeviceDarwin_h
#define EagleGlimpse_PuckDeviceDarwin_h

// Only Valid for OS X

#include <mach/mach.h>
#include <CoreFoundation/CFNumber.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/hid/IOHIDManager.h>

#include <functional>

#include <gadget/gadgetParam.h>
#include <gadget/Type/InputBaseTypes.h>
#include <gadget/Type/Input.h>
#include <gadget/Type/Digital.h>
#include <gadget/Type/Analog.h>
#include <gadget/InputManager.h>

namespace SpaceBall { namespace Io { namespace Hid {
    class DeviceDictionary
    {
      public:
        DeviceDictionary();
        ~DeviceDictionary();
        void add(const char* key, int value);
        CFMutableDictionaryRef ref() { return _ref; }
      private:
        CFMutableDictionaryRef _ref;
    };

    class DeviceDictionaryArray
    {
      public:
        DeviceDictionaryArray();
        ~DeviceDictionaryArray();
        CFArrayRef ref() { return _ref; }
        void add(int vendorId, int productId);
      private:
        CFMutableArrayRef _ref;
    };

    class Device
    {
      public:
        Device(IOHIDDeviceRef ref) : _ref(ref) {}
        ~Device() {}

        std::string getProperty(const char *name);
      private:
        IOHIDDeviceRef _ref;
    };

    class Value
    {
      public:
        Value(IOHIDValueRef ref) : _ref(ref) { _ele = IOHIDValueGetElement(_ref); }
        uint32_t reportId() { return IOHIDElementGetReportID(_ele); }
        uint32_t usage() { return IOHIDElementGetUsage(_ele); }

        CFIndex byteCount() { return IOHIDValueGetLength(_ref); }
        const uint8_t * bytes() { return IOHIDValueGetBytePtr(_ref); }
        short asShort() { return reinterpret_cast<const short *>(bytes())[0]; }
        bool  asBool() { return bytes()[0]; }

      private:
        IOHIDValueRef _ref;
        IOHIDElementRef _ele;
    };

    class Manager 
    {
      public:
        Manager();
        ~Manager();

        void setDeviceMatching(int vendorId, int productId);
        void setDeviceMatching(DeviceDictionaryArray &list);

        void run();
        void stop();

        void onMatching(boost::function<void(Device &)> func) { _onMatching = func; }
        void onRemoval(boost::function<void(Device &)> func) { _onRemoval = func; }
        void onInputValue(boost::function<void(Value &)> func) { _onInputValue = func; }
        
      private:
        IOHIDManagerRef _ref;

        void printMatchingDevice(Device &device);

        void onMatchingPrivate(Device &device)
            {
                printMatchingDevice(device);
                if (_onMatching) _onMatching(device);
            }
        void onRemovalPrivate(Device &device)
            {
                std::cout << "[PuckDevice-Darwin] Lost a Puck Device" << std::endl;
                if (_onRemoval) _onRemoval(device);
            }
        void onInputValuePrivate(Value &value) { if (_onInputValue) _onInputValue(value); }

        static void onMatchingThunk(void *inContext, IOReturn inResult, void *inSender, IOHIDDeviceRef inIOHIDDeviceRef);
        static void onRemovalThunk(void *inContext, IOReturn inResult, void *inSender, IOHIDDeviceRef inIOHIDDeviceRef);
        static void onInputValueThunk(void *inContext, IOReturn inResult, void *inSender, IOHIDValueRef inIOHIDValueRef);

        boost::function<void(Device &)> _onMatching;
        boost::function<void(Device &)> _onRemoval;
        boost::function<void(Value &)> _onInputValue;
    };
}
}
}


/// OS X Drivers for 3DConnexion Devices
class PuckDeviceDarwin : public gadget::input_digital_analog_t
{
public:
    typedef void* USBHIDHandle;

    /// Constructor
    PuckDeviceDarwin();
    /// Destructor
    virtual ~PuckDeviceDarwin();

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
    SpaceBall::Io::Hid::Manager *_manager;

    void onRemoval(SpaceBall::Io::Hid::Device &);
    void onInputValue(SpaceBall::Io::Hid::Value &value);

    /// Initialize the analog and digital buffers
    void initBuffers();

    /// Digital data to be sent to gadget classes
    std::vector<gadget::DigitalData> _buttons;
    /// Analog data to be sent to gadget classes
    std::vector<gadget::AnalogData> _axis;
    ///
    int _axesCount[6];
    
    float normalize(const float in)
    { float out; gadget::Analog::normalizeMinToMax(in, out); return out; }
};

#endif
