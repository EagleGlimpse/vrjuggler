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


package VjComponents.UI;

import javax.swing.*;

import VjControl.Core;
import VjComponents.UI.PlugPanel;
import VjComponents.UI.PlugPanelContainer;
import VjConfig.*;

public class TabPaneContainer
    extends JTabbedPane 
    implements PlugPanelContainer {

    public String component_name;
    public ConfigChunk component_chunk;

    public TabPaneContainer () {
        super();
        component_name = "Unconfigured TabPaneContainer";
        component_chunk = null;
    }

    //---------------------- PlugPanelContainer Stuff ------------------------

    public void insertPanel (PlugPanel p) {
//                  int index = -1;
//                  if (tabs_config == null)
//                      index = tabpane.getComponentCount();
//                  else {
//                      for (int i = 0; (i < tabs_config.length) && (index == -1); i++)
//                          if (tabs_config[i].equalsIgnoreCase(ch.getName()))
//                              index = 0; // tmp kludge
//                  }
        try {
            int index = 0;
            insertTab (p.getComponentName(), 
                       p.getComponentIcon(), 
                       (JComponent)p, 
                       null, index);
        }
        catch (ClassCastException e) {
            Core.consoleErrorMessage ("GUI", "PlugPanel '" + 
                                      p.getComponentName() +
                                      "' isn't a JComponent!");
        }
    }



    public void removePanel (PlugPanel p) {
        try {
            remove ((JComponent)p);
        }
        catch (ClassCastException e) {
            Core.consoleErrorMessage ("GUI", "PlugPanel '" + 
                                      p.getComponentName() +
                                      "' isn't a JComponent!");
        }
    }


    public void rebuildDisplays () {
        PlugPanel p;
        for (int i = 0; i < getTabCount(); i++) {
            p = (PlugPanel)getComponentAt(i);
            p.rebuildDisplay();
        }
    }

    //----------------------- VjComponent Stuff ------------------------------


    public String getComponentName () {
        return component_name;
    }


    public boolean configure (ConfigChunk ch) {
        component_chunk = ch;
        component_name = ch.getName();
        return true;
    }


    public ConfigChunk getConfiguration () {
        return component_chunk;
    }


    public boolean addConfig (ConfigChunk ch) {
        return false;
    }


    public boolean removeConfig (String name) {
        return false;
    }


    public void destroy () {
        ;
    }


}
