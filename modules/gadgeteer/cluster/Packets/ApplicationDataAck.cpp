/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2003 by Iowa State University
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

//#include <gadget/gadgetConfig.h>
#include <cluster/Packets/ApplicationDataAck.h>
#include <cluster/ClusterNetwork/ClusterNetwork.h>
#include <cluster/ClusterNetwork/ClusterNode.h>
#include <cluster/ClusterManager.h>
#include <cluster/Plugins/ApplicationDataManager/ApplicationDataManager.h>
#include <gadget/InputManager.h>
#include <jccl/RTRC/ConfigManager.h>
#include <gadget/Util/Debug.h>

namespace cluster
{
   ApplicationDataAck::ApplicationDataAck(Header* packet_head, vpr::SocketStream* stream)
   {
      recv(packet_head,stream);
      parse();
   }
   ApplicationDataAck::ApplicationDataAck(const vpr::GUID& plugin_guid, 
                                          const vpr::GUID& id, 
                                          const bool ack)
   {
      // Given the input, create the packet and then serialize the packet(which includes the header)
      // - Set member variables
      // - Create the correct packet header
      // - Serialize the packet

      // =============== Packet Specific =================
      //

      // Device Request Vars
      mPluginId = plugin_guid;
      mId = id;
      mAck = ack;
      
      // string -> string.size()
      // Uint8 -> 1
      // Uint16 -> 2
      // Uint32 -> 4

      // 2 + 2 + 2 + mDeviceName.size() + 2 + mDeviceBaseType() + 1

      // Header vars (Create Header)
      mHeader = new Header(Header::RIM_PACKET,
                                      Header::RIM_APPDATA_ACK,
                                      Header::RIM_PACKET_HEAD_SIZE 
                                      + 16/*Plugin GUID*/
                                      + 16/*Object GUID*/
                                      + 1 /*mAck*/,
                                      0);                      
      //
      // =============== Packet Specific =================

      serialize();
   }
   void ApplicationDataAck::serialize()
   {
      // - Clear
      // - Write Header
      // - Write data
      
      mPacketWriter->getData()->clear();
      mPacketWriter->setCurPos(0);

      // Create the header information
      mHeader->serializeHeader();
      
      // =============== Packet Specific =================
      //
      
      // Serialize plugin GUID
      mPluginId.writeObject(mPacketWriter);

      // Serialize object GUID
      mId.writeObject(mPacketWriter);

      // Ack
      mPacketWriter->writeBool(mAck);

      //
      // =============== Packet Specific =================
   }
   void ApplicationDataAck::parse()
   {
      // =============== Packet Specific =================
      //
      
      // De-Serialize plugin GUID
      mPluginId.readObject(mPacketReader);
      
      // De-Serialize object GUID
      mId.readObject(mPacketReader);

      // Ack
      mAck = mPacketReader->readBool();

      //
      // =============== Packet Specific =================
   }

   bool ApplicationDataAck::action(ClusterNode* node)
   {  
      return false;
   }


   void ApplicationDataAck::printData(int debug_level)
   {
      vprDEBUG_BEGIN(gadgetDBG_RIM,debug_level) 
         <<  clrOutBOLD(clrYELLOW,"==== ApplicationData Ack Packet ====\n") << vprDEBUG_FLUSH;
      
      Packet::printData(debug_level);

      vprDEBUG(gadgetDBG_RIM,debug_level) 
         << clrOutBOLD(clrYELLOW, "Plugin GUID:        ") << mPluginId.toString()
         << std::endl << vprDEBUG_FLUSH;
      vprDEBUG(gadgetDBG_RIM,debug_level) 
         << clrOutBOLD(clrYELLOW, "Object GUID:        ") << mId.toString()
         << std::endl << vprDEBUG_FLUSH;
      vprDEBUG(gadgetDBG_RIM,debug_level) 
         << clrOutBOLD(clrYELLOW, "Ack or Nack:      ") << (mAck ? "Ack" : "Nack")  << std::endl
         << std::endl << vprDEBUG_FLUSH;

      vprDEBUG_END(gadgetDBG_RIM,debug_level) 
         <<  clrOutBOLD(clrYELLOW,"================================\n") << vprDEBUG_FLUSH;      
   }

}   // end namespace gadget
