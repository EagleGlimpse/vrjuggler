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


package VjComponents.ConfigEditor;

import java.io.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

import VjControl.Core;
import VjControl.VjComponent;
import VjConfig.*;
import VjComponents.UI.*;
import VjComponents.UI.PlugPanel;
import VjComponents.UI.ControlUIModule;

/** PlugPanel for editing a pair of ConfigChunkDB files side-by-side.
 * 
 *  @author Christopher Just
 *  @version $Revision$
 */
public class ConfigurePane extends JSplitPane 
    implements PlugPanel, MouseListener {

    ChunkDBPanel leftpanel, rightpanel;
    protected String component_name;
    protected ConfigChunk component_chunk;

    protected ControlUIModule ui_module;
    protected ConfigModule config_module;
    protected ConfigUIHelper confighelper_module;

    public ConfigurePane() {
	super(JSplitPane.HORIZONTAL_SPLIT);
	
        ui_module = null;
        config_module = null;
        confighelper_module = null;

        component_name = "Unconfigured ConfigurePane";
        component_chunk = null;

	//-------------------- BUILD CONFIGURATION PANE ---------------------

// 	setLeftComponent (leftpanel = new ChunkDBPanel(0));
// 	setRightComponent (rightpanel = new ChunkDBPanel(1));

// 	leftpanel.setMinimumSize (new Dimension (0, 0));
// 	rightpanel.setMinimumSize (new Dimension (0, 0));
// 	setDividerLocation (0.5d);
//         setOneTouchExpandable (true);

// 	leftpanel.setSendAcrossTarget (rightpanel);
// 	rightpanel.setSendAcrossTarget (leftpanel);
	
// 	leftpanel.addMouseListener (leftpanel);
// 	rightpanel.addMouseListener (rightpanel);
	
    }



    public void selectLeftDB (String name) {
	leftpanel.selectDB (name);
    }

    public void selectRightDB (String name) {
	rightpanel.selectDB (name);
    }


//      public String getLeftDB () {
//          leftpanel.getDBName();
//      }

//      public String getRightDB () {
//          rightpanel.getDBName();
//      }


    // MouseListener stuff
    public void mouseClicked(MouseEvent e) {
	if (e.getSource() == leftpanel)
	    System.out.println ("left panel click");
	else
	    System.out.println ("right panel click");
    }

    public void mouseEntered(MouseEvent e) {}
    public void mouseExited(MouseEvent e) {}
    public void mousePressed(MouseEvent e) {}
    public void mouseReleased(MouseEvent e) {}



    /********************** PlugPanel Stuff **************************/

    public String getComponentName () {
        return component_name;
    }


    public ImageIcon getComponentIcon () {
        //ControlUIModule ui_module = (ControlUIModule)Core.getComponentFromRegistry ("ControlUI Module");
        if (ui_module != null) {
            return ui_module.getIcon ("open file", 0);
        }
        else
            return null;
    }


    public boolean configure (ConfigChunk ch) {
        component_chunk = ch;
        component_name = ch.getName();

        // get pointers to the modules we need.
        Property p = ch.getPropertyFromToken ("Dependencies");
        if (p != null) {
            int i;
            int n = p.getNum();
            String s;
            VjComponent c;
            for (i = 0; i < n; i++) {
                s = p.getValue(i).toString();
                c = Core.getComponentFromRegistry(s);
                if (c != null) {
                    if (c instanceof ControlUIModule)
                        ui_module = (ControlUIModule)c;
                    if (c instanceof ConfigModule)
                        config_module = (ConfigModule)c;
                    if (c instanceof ConfigUIHelper)
                        confighelper_module = (ConfigUIHelper)c;
                }
            }
        }
        if (ui_module == null)
            ui_module = (ControlUIModule)Core.getComponentFromRegistry ("ControlUI Module");
        if (config_module == null)
            config_module = (ConfigModule)Core.getComponentFromRegistry ("Config Module");
        if (confighelper_module == null)
            confighelper_module = (ConfigUIHelper)Core.getComponentFromRegistry ("ConfigUIHelper Module");
        if ((ui_module == null) || (config_module == null) || (confighelper_module == null)) {
            Core.consoleErrorMessage (component_name, "Instantiated with unmet VjComponent Dependencies. Fatal Configuration Error!");
            return false;
        }


	setLeftComponent (leftpanel = new ChunkDBPanel(0));
	setRightComponent (rightpanel = new ChunkDBPanel(1));

        leftpanel.setComponentName ("Left ConfigurePane");
        leftpanel.setControlUIModule (ui_module);
        leftpanel.setConfigModule (config_module);
        leftpanel.setConfigUIHelper (confighelper_module);
        leftpanel.initialize();

        rightpanel.setComponentName ("Right ConfigurePane");
        rightpanel.setControlUIModule (ui_module);
        rightpanel.setConfigModule (config_module);
        rightpanel.setConfigUIHelper (confighelper_module);
        rightpanel.initialize();

	leftpanel.setMinimumSize (new Dimension (0, 0));
	rightpanel.setMinimumSize (new Dimension (0, 0));
	setDividerLocation (0.5d);
        setOneTouchExpandable (true);

	leftpanel.setSendAcrossTarget (rightpanel);
	rightpanel.setSendAcrossTarget (leftpanel);
	
	leftpanel.addMouseListener (leftpanel);
	rightpanel.addMouseListener (rightpanel);



        return true;
    }


    public ConfigChunk getConfiguration () {
        return component_chunk;
    }


    public boolean addConfig (ConfigChunk ch) {
        return false;
    }


    public boolean removeConfig (String s) {
        return false;
    }


    public void destroy () {
	leftpanel.destroy();
	rightpanel.destroy();
    }


    public void rebuildDisplay () {
        leftpanel.rebuildDisplay();
        rightpanel.rebuildDisplay();
    }


}



