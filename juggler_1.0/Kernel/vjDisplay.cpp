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

#include <Kernel/vjDisplay.h>
#include <Kernel/vjViewport.h>
#include <Kernel/vjSimViewport.h>
#include <Kernel/vjSurfaceViewport.h>

void vjDisplay::updateProjections()
{
   for(unsigned i=0;i<mViewports.size();i++)
   {
      mViewports[i]->updateProjections();
   }
}


void vjDisplay::config(vjConfigChunk* chunk)
{
   vjASSERT(chunk != NULL);

   configDisplayWindow(chunk);
   configViewports(chunk);
}

void vjDisplay::configDisplayWindow(vjConfigChunk* chunk)
{
   vjASSERT(chunk != NULL);

   // -- Get config info from chunk -- //
    int originX = chunk->getProperty("origin", 0);
    int originY = chunk->getProperty("origin", 1);
    int sizeX   = chunk->getProperty("size", 0);
    int sizeY   = chunk->getProperty("size", 1);
    std::string name  = chunk->getProperty("name");
    mBorder     = chunk->getProperty("border");
    int pipe    = chunk->getProperty("pipe");
    mActive  = chunk->getProperty("active");

   // -- Check for error in configuration -- //
   // NOTE: If there are errors, set them to some default value
   if(sizeX <= 0)
   {
      vjDEBUG(vjDBG_DISP_MGR,2) << "WARNING: window sizeX set to: " << sizeX
                                << ".  Setting to 10." << std::endl
                                << vjDEBUG_FLUSH;
      sizeX = 10;
   }

   if(sizeY <= 0)
   {
      vjDEBUG(vjDBG_DISP_MGR,2) << "WARNING: window sizeY set to: " << sizeY
                                << ".  Setting to 10." << std::endl
                                << vjDEBUG_FLUSH;
      sizeY = 10;
   }

   if(pipe < 0)
   {
      vjDEBUG(vjDBG_DISP_MGR,2) << "WARNING: pipe was negative, pipe set to: " << pipe << ".  Setting to 0.\n" << vjDEBUG_FLUSH;
      pipe = 0;
   }

      // -- Set local window attributes --- //
    setOriginAndSize(originX, originY, sizeX, sizeY);

    setName(name);
    setPipe(pipe);

    mDisplayChunk = chunk;        // Save the chunk for later use
}

void vjDisplay::configViewports(vjConfigChunk* chunk)
{
   vjASSERT(chunk != NULL);

   unsigned num_sim_vps = chunk->getNum("sim_viewports");
   unsigned num_surface_vps = chunk->getNum("surface_viewports");

   vjConfigChunk* vp_chunk = NULL;
   vjSimViewport* sim_vp = NULL;
   vjSurfaceViewport* surf_vp = NULL;

   unsigned i(0);

   // Create sim viewports
   for(i=0;i<num_sim_vps;i++)
   {
      vp_chunk = chunk->getProperty("sim_viewports",i);
      sim_vp = new vjSimViewport;
      sim_vp->config(vp_chunk);
      mViewports.push_back(sim_vp);
   }

   // Create sim viewports
   for(i=0;i<num_surface_vps;i++)
   {
      vp_chunk = chunk->getProperty("surface_viewports",i);
      surf_vp = new vjSurfaceViewport;
      surf_vp->config(vp_chunk);
      mViewports.push_back(surf_vp);
   }
}


std::ostream& operator<<(std::ostream& out, vjDisplay& disp)
{
   out << std::setw(15) << disp.mName.c_str() << std::endl
        << "  org:" << disp._xo << ", " << disp._yo
        << "  sz:" << disp._xs << ", " << disp._ys
        << "  p:" << disp.mPipe
        << "  act:" << (disp.mActive ? "Y" : "N");
   for(unsigned i=0;i<disp.mViewports.size();i++)
   {
      out << "vp: " << i << " " << *(disp.mViewports[i]) << std::endl;
   }

   return out;
}
