/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2005 by Iowa State University
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

package org.vrjuggler.vrjconfig.commoneditors.devicegraph;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.jgraph.graph.DefaultGraphCell;

import org.vrjuggler.jccl.config.ConfigContext;
import org.vrjuggler.jccl.config.ConfigElement;
import org.vrjuggler.jccl.config.PropertyDefinition;

import org.vrjuggler.vrjconfig.commoneditors.EditorConstants;
import org.vrjuggler.vrjconfig.commoneditors.devicegraph.extras.SimAnalogUnitPropertyHandler;


/**
 * The default device graph cell creator for the input device config elements
 * that ship with VR Juggler by default.
 */
public class DefaultDeviceGraphCellCreator
   implements DeviceGraphCellCreator
            , EditorConstants
{
   public DefaultGraphCell createDeviceGraphCell(ConfigElement devElt,
                                                 ConfigContext context,
                                                 Map attributes)
   {
      DefaultGraphCell cell = null;

      // XXX: Come up with a better system for setting the x and y values.
      int x = 120, y = 20;

      String token = devElt.getDefinition().getToken();

      // These device types all have exactly one unit.  In this case, the
      // unit type is irrelevant.
      if ( token.equals(SIM_POS_DEVICE_TYPE) ||
           token.equals(KEYBOARD_MOUSE_DEVICE_TYPE) ||
           token.equals(TRACKD_CONTROLLER_TYPE) ||
           token.equals(TRACKD_SENSOR_TYPE) ||
           token.equals(TRACKD_API_CONTROLLER_TYPE) ||
           token.equals(TRACKD_API_SENSOR_TYPE) )
      {
         cell = GraphHelpers.createDeviceCell(devElt, context, 1, attributes,
                                              x, y, true);
      }
      else if ( token.equals(SIM_ANALOG_DEVICE_TYPE) )
      {
         Map unit_types = new HashMap();
         unit_types.put(UnitConstants.ANALOG, null);
         SimAnalogUnitPropertyHandler h = new SimAnalogUnitPropertyHandler();
         cell = GraphHelpers.createBaseDeviceCell(
            new DeviceInfo(devElt, context, unit_types, h), attributes, x, y,
            false
         );

         int dec_count =
            devElt.getPropertyValueCount(DECREMENT_KEYPRESS_PROPERTY);
         int inc_count =
            devElt.getPropertyValueCount(INCREMENT_KEYPRESS_PROPERTY);

         if ( dec_count != inc_count )
         {
            throw new IllegalArgumentException(
               "Increment/decrement keypress mismatch in " + devElt.getName() +
               ": " + inc_count + " != " + dec_count
            );
         }

         GraphHelpers.addDevicePorts(cell, UnitConstants.ANALOG, inc_count);
      }
      // A simulated digital device has a variable number of digital inputs
      // based on a variable-valued property.
      else if ( token.equals(SIM_DIGITAL_DEVICE_TYPE) )
      {
         cell = GraphHelpers.createDeviceCell(devElt, context,
                                              KEY_PAIR_PROPERTY, attributes,
                                              x, y, false);
      }
      // The Flock of Birds and MotionStar Wireless have a variable number of
      // positional units that are determined at run time by the respective
      // drivers.
      else if ( token.equals(FLOCK_TYPE) || token.equals(MOTION_STAR_TYPE) )
      {
         List proxy_types = new ArrayList(1);
         proxy_types.add(0, POSITION_PROXY_TYPE);
         cell = GraphHelpers.createDeviceCell(devElt, context, proxy_types,
                                              attributes, x, y, false);
      }
      // The IS-900 and IntersenseAPI drivers have a variable number of
      // properties...
      // XXX: This is not complete because it does not take into account hte
      // untis types.  Adding and removing units with an Intersense is hard...
      else if ( token.equals(INTERSENSE_TYPE) ||
                token.equals(INTERSENSE_API_TYPE) )
      {
         cell = GraphHelpers.createDeviceCell(devElt, context,
                                              STATIONS_PROPERTY, attributes,
                                              x, y, false);
      }
      // The Linux and Direct Input game controllers have a variable number of
      // analog and digital units.  The number is dependent upon the game
      // controller and is determined at run-time by the driver.
      else if ( token.equals(DIRECTX_JOYSTICK_TYPE) ||
                token.equals(LINUX_JOYDEV_TYPE) )
      {
         List proxy_types = new ArrayList(2);
         proxy_types.add(0, ANALOG_PROXY_TYPE);
         proxy_types.add(1, DIGITAL_PROXY_TYPE);
         cell = GraphHelpers.createDeviceCell(devElt, context, proxy_types,
                                              attributes, x, y, false);
      }
      else if ( token.equals(PUCK_DEVICE_TYPE) )
      {
      }
      // The Ibox has a fixed number of analog and digital units (4 of both).
      else if ( token.equals(IBOX_TYPE) )
      {
         List unit_list = new ArrayList(2);
         unit_list.add(0, UnitConstants.ANALOG);
         unit_list.add(1, UnitConstants.DIGITAL);
         cell = GraphHelpers.createBaseDeviceCell(
            new DeviceInfo(devElt, context, unit_list), attributes, x, y, false
         );
         GraphHelpers.addDevicePorts(cell, UnitConstants.ANALOG, 4);
         GraphHelpers.addDevicePorts(cell, UnitConstants.DIGITAL, 4);
      }
      else if ( token.equals(DATA_GLOVE_TYPE) )
      {
      }
      // The Pinch Gloves have a fixed numbmer of digital untis (11).
      else if ( token.equals(PINCH_GLOVE_TYPE) )
      {
         List unit_list = new ArrayList(1);
         unit_list.add(0, UnitConstants.DIGITAL);
         cell = GraphHelpers.createBaseDeviceCell(
            new DeviceInfo(devElt, context, unit_list), attributes, x, y, false
         );
         GraphHelpers.addDevicePorts(cell, UnitConstants.DIGITAL, 11);
      }
      // VRPN has a variable number of digital and positional units defined
      // by two property values.
      else if ( token.equals(VRPN_TYPE) )
      {
         Integer b_count =
            (Integer) devElt.getProperty(BUTTON_COUNT_PROPERTY, 0);
         Integer t_count =
            (Integer) devElt.getProperty(TRACKER_COUNT_PROPERTY, 0);

         PropertyDefinition b_prop_def =
            devElt.getDefinition().getPropertyDefinition(BUTTON_COUNT_PROPERTY);
         PropertyDefinition t_prop_def =
            devElt.getDefinition().getPropertyDefinition(TRACKER_COUNT_PROPERTY);

         Map unit_map = new HashMap();
         unit_map.put(UnitConstants.DIGITAL, b_prop_def);
         unit_map.put(UnitConstants.POSITION, t_prop_def);
         cell = GraphHelpers.createBaseDeviceCell(
            new DeviceInfo(devElt, context, unit_map), attributes, x, y, false
         );
         GraphHelpers.addDevicePorts(cell, UnitConstants.DIGITAL,
                                     b_count.intValue());
         GraphHelpers.addDevicePorts(cell, UnitConstants.POSITION,
                                     t_count.intValue());
      }

      return cell;
   }
}
