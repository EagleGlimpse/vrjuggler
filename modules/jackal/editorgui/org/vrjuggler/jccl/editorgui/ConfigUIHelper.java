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


import java.awt.event.*;
import java.net.URL;
import javax.swing.*;
import javax.swing.event.*;
import java.util.Vector;
import java.util.HashMap;

import VjComponents.UI.*;
import VjComponents.UI.Widgets.ChildFrame;
import VjComponents.UI.Widgets.ChildFrameParent;
import VjComponents.UI.Widgets.GenericEditorFrame;
import VjComponents.UI.Widgets.EasyMenuBar;
import VjComponents.ConfigEditor.ChunkDescUI.ChunkDescPanel;
import VjControl.Core;
import VjControl.CoreModule;
import VjControl.VjComponent;
import VjComponents.ConfigEditor.ConfigChunkPanel;
import VjControl.DefaultCoreModule;
import VjComponents.UI.EditorPanel;
import VjComponents.ConfigEditor.ConfigModule;
import VjComponents.ConfigEditor.ConfigModuleEvent;
import VjComponents.ConfigEditor.ConfigModuleListener;
import VjConfig.*;



/** Helper for dealing with ConfigChunk and ChunkDesc files.
 *  This provides a variety of services:
 *      - ConfigChunkFrames
 *      - ChunkDescFrames
 *      - ConfigChunk Dependency Frames
 *      - ChunkDBTreeModelFactory
 *      - DescHelp HTMLFrames
 *  <p>
 *  These are in a separate CoreModule because they might be needed by
 *  a number of UI components - most obviously the current ChunkDBPanels,
 *  but also by some of the config-autogeneration panelss.
 *  <p>
 *  Previously, a lot of the functionality for handling ConfigChunkFrames
 *  was in ControlUIModule, create an ugly dependency and wrecking 
 *  genericity.
 */
public class ConfigUIHelper
    extends DefaultCoreModule
    implements ActionListener,
               ConfigModuleListener,
               ChildFrameParent,
               DescDBListener {


    public class ConfigChunkPanelFactory {

        protected HashMap chunkpanel_mappings;

        public ConfigChunkPanelFactory () {
            chunkpanel_mappings = new HashMap();
        }


        /** Add a new specialized ChunkPanel component to the factory.
         *  @bug If configuration fails, some map entries may be left in
         *       place.
         */
        public boolean addConfig (ConfigChunk ch) {
            try {
                String cn = ch.getValueFromToken ("ClassName", 0).getString();
                Property p = ch.getPropertyFromToken ("UsedFor");
                if (p == null)
                    return false;
                int n = p.getNum();
                String s;
                for (int i = 0; i < n; i++) {
                    s = p.getValue(i).toString();
                    chunkpanel_mappings.put (s, cn);
                }
                return true;
            }
            catch (Exception e) {
                return false;
            }
        }


        public ConfigChunkPanel createConfigChunkPanel (String desc_token) {
            ConfigChunkPanel p = null;
            try {
                String cn = (String)chunkpanel_mappings.get(desc_token);
                if (cn == null)
                    cn = "VjComponents.ConfigEditor.ConfigChunkUI.DefaultConfigChunkPanel";
                p = (ConfigChunkPanel)Core.component_factory.createComponent(cn);
                return p;
            }
            catch (Exception e) {
                System.out.println ("error in ConfigChunkPanelFactory.createConfigChunkPanel");
                return null;
            }
        }


    } // class ConfigChunkPanelFactory



    public ControlUIModule ui_module;
    public ConfigModule config_module;

    public  ConfigChunkPanelFactory configchunkpanel_factory;
    public  ChunkDBTreeModelFactory chunkdbt_factory;

    /** All the frames (ConfigChunkFrames, mainly, but not exclusively). */
    Vector child_frames;

    protected JMenuItem rebuildtrees_mi;


    public ConfigUIHelper () {
        
        component_name = "Unconfigured ConfigUIHelper";
        child_frames = new Vector();

        ui_module = null;
        config_module = null;

        configchunkpanel_factory = new ConfigChunkPanelFactory();
        chunkdbt_factory = new ChunkDBTreeModelFactory();

    }



    public boolean configure (ConfigChunk ch) {
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
                }
            }
        }
        if (ui_module == null)
            ui_module = (ControlUIModule)Core.getComponentFromRegistry ("ControlUI Module");
        if (config_module == null)
            config_module = (ConfigModule)Core.getComponentFromRegistry ("Config Module");
        if ((ui_module == null) || (config_module == null)) {
            Core.consoleErrorMessage (component_name, "Instantiated with unmet VjComponent Dependencies. Fatal Configuration Error!");
            return false;
        }


//         if (ui_module == null) {
//             Core.consoleErrorMessage (component_name, "ConfigUIHelper expected ControlUI Module to exist.");
//             return false;
//         }
//         config_module = (ConfigModule)Core.getModule ("Config Module");
//         if (config_module == null) {
//             Core.consoleErrorMessage (component_name, "ControlUI expected Config Module to exist.");
//             return false;
//         }

        config_module.addConfigModuleListener (this);
 	config_module.descdb.addDescDBListener (this);


        // Menu stuff

        EasyMenuBar menubar = ui_module.getEasyMenuBar();

        rebuildtrees_mi = menubar.addMenuItem ("Edit/Rebuild ChunkDB Trees");
        rebuildtrees_mi.addActionListener (this);

	// initial helpdesc menu stuff...
        JMenuItem newmenu;
        for (int i = 0; i < config_module.descdb.size(); i++) {
            ChunkDesc d = (ChunkDesc)config_module.descdb.elementAt(i);
            newmenu = menubar.addMenuItem ("Help/Chunk-Specific Help/" + d.getName());
            newmenu.addActionListener(this);
        }

        return true;
    }


    public void destroy () {
	config_module.removeConfigModuleListener (this);
	config_module.descdb.removeDescDBListener (this);
    }


    //----------------- ChunkDBTreeModel Stuff -------------------


    public ChunkDBTreeModel getChunkDBTreeModel (ConfigChunkDB chunkdb) {
        return chunkdbt_factory.getTreeModel (chunkdb);
    }

    public void releaseChunkDBTreeModel (ChunkDBTreeModel dbt) {
        chunkdbt_factory.releaseTreeModel (dbt);
    }


    //------------------- DescHelp Stuff ---------------------------

    /** utility for loadDescHelp() */
    private URL getFileURL (String fname) {
	URL url = null;
	try {
	    url = new URL ("file", "localhost", -1, fname);
	    // make sure it's really there.
	    if (url.openConnection().getContentLength() == -1)
		url = null;
	}
	catch (Exception e) {
	    url = null;
	}
	return url;
    }



    public void loadDescHelp (String s) {
        String url = "DescHelp/" + s + ".html";
	HTMLFrame help_frame = new HTMLFrame (this, "VjControl Help", null);
        help_frame.setURL (url);
        help_frame.setContentsURL (ClassLoader.getSystemResource ("VjFiles/VjControlIndex.html"));
	child_frames.addElement (help_frame);
	help_frame.show();
    }


    //---------------------------

    public boolean addConfig (ConfigChunk ch) {

        try {
            String classname = ch.getValueFromToken ("classname", 0).getString();
            if (Core.component_factory.classSupportsInterface (classname, "VjComponents.ConfigEditor.ConfigChunkPanel")) {
                configchunkpanel_factory.addConfig (ch);
                // don't register w/ core cuz we didn't instantiate anything
                return true;
            }
            else {
                Core.consoleErrorMessage ("ControlUI", "Unrecognized component: " + classname);
                return false;
            }
        }
        catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }



    /** Open a frame for editing the ConfigChunk ch, which is part of chunkdb.
     *  The frame will be managed & owned by self, which will also take care
     *  of notifications/modifications when the frame is closed.
     */
    public void openChunkFrame (ConfigChunkDB chunkdb, ConfigChunk ch) {
	if (ch == null)
	    return;
	GenericEditorFrame f = (GenericEditorFrame)getChildFrameMatching ("VjComponents.ConfigEditor.ConfigChunkPanel", chunkdb, ch);
	if (f == null) {
            ConfigChunkPanel p = configchunkpanel_factory.createConfigChunkPanel (ch.getDescToken());
            if (p != null) {
                p.setChunk (ch, chunkdb);
                f = new GenericEditorFrame (this, p);
                addChildFrame (f);
	    }
            else
                Core.consoleErrorMessage (component_name, "Failed to create ConfigChunk editor panel");
	}
	else
	    f.show();
    }



    public void openDescFrame (ChunkDescDB db, ChunkDesc desc, boolean editable) {
        if (desc == null)
            return;
        GenericEditorFrame f = (GenericEditorFrame)getChildFrameMatching("VjComponents.ConfigEditor.chunkdesc.ChunkDescPanel", db, desc);
        if (f == null) {
            ChunkDescPanel p = new ChunkDescPanel (desc, db, editable);
            if (p != null) {
                f = new GenericEditorFrame (this, p);
                addChildFrame (f);
	    }
            else
                Core.consoleErrorMessage (component_name, "Failed to create ChunkDesc editor panel");
        }
        else
            f.show();
    }



    /** Callback when one of ConfigUIHelper's children is closed.
     */
    public void closeChild (ChildFrame frame) {
	if (frame instanceof GenericEditorFrame) {
	    child_frames.removeElement(frame);
	    frame.destroy();
	}
        else if (frame instanceof HTMLFrame) {
            child_frames.removeElement (frame);
            frame.destroy();
        }
        else 
            System.out.println ("totally unidentified child frame");
    }


    /**
     * Callback when one of ControlUI's children is applied (this sucks)
     */
    public void applyChild (ChildFrame frame) {
	if (frame instanceof GenericEditorFrame) {
            EditorPanel ep = ((GenericEditorFrame)frame).getEditorPanel();
            if (ep instanceof ConfigChunkPanel) {
                ConfigChunkPanel p = (ConfigChunkPanel)ep;
                ConfigChunk newc, oldc;
                oldc = p.getOldValue();
                newc = p.getNewValue();
                ConfigChunkDB chunkdb = p.getChunkDB();
                config_module.replaceChunk (chunkdb, oldc, newc);
            }
            else if (ep instanceof ChunkDescPanel) {
                ChunkDescPanel dp = (ChunkDescPanel)ep;
                ChunkDesc oldc = dp.getOldValue();
                ChunkDesc newc = dp.getNewValue();
                ChunkDescDB descdb = dp.getDescDB();
                config_module.replaceDesc (descdb, oldc, newc);
            }
	}
        else
            System.out.println ("Don't know what to do with this frame...");
    }



    /** Adds a child frame (e.g. a ConfigChunkFrame or HTMLFrame).
     */
    protected void addChildFrame (ChildFrame f) {
	child_frames.addElement(f);
    }


    protected void removeChildFrame (ChildFrame f) {
	child_frames.removeElement (f);
	f.destroy();
    }



    protected void removeChildFramesMatching (String cl, Object db, Object o) {
	ChildFrame f;
	for (int i = 0; i < child_frames.size(); ) {
	    f = (ChildFrame)child_frames.elementAt(i);
	    if (f.matches (cl, db, o)) {
		child_frames.removeElementAt(i);
		f.destroy();
	    }
	    else
		i++;
	}
    }



    protected ChildFrame getChildFrameMatching (String cl, Object db, Object o) {
	ChildFrame f;
	for (int i = 0; i < child_frames.size(); i++) {
	    f = (ChildFrame)child_frames.elementAt(i);
	    if (f.matches (cl, db, o))
		return f;
	}
	return null;
    }



    //------------------------ ActionListener Stuff --------------------------

    public void actionPerformed (ActionEvent e) {
        Object o = e.getSource();
        if (o == rebuildtrees_mi) {
            // using rebuildAllPanels here is somewhat kludgey but i don't
            // have a better interface right now.
            chunkdbt_factory.removeAllElements();
            ui_module.rebuildAllPanels();
        }
	else if (o instanceof JMenuItem) {
	    // the default case - we'll assume the menuitem is a chunk-
	    // specific help request. it's name will be a chunkdesc name
	    String name = ((JMenuItem)o).getText();
	    ChunkDesc d = config_module.descdb.getByName (name);
	    if (d != null)
		loadDescHelp (d.getToken());
	}
        else
            Core.consoleErrorMessage ("ConfigUIHelper", "actionPerformed is confused by its arguments.");
    }



    //--------------------- ConfigModuleListener Stuff -----------------------

    // These are mostly in here so we can close child frames when an 
    // associated db is unloaded.
    public void addChunkDB (ConfigModuleEvent e) {
    }
    public void removeChunkDB (ConfigModuleEvent e) {
	removeChildFramesMatching (null, e.getChunkDB(), null);
    }
    public void addDescDB (ConfigModuleEvent e) {
    }
    public void removeDescDB (ConfigModuleEvent e) {
	removeChildFramesMatching (null, e.getDescDB(), null);
    }


    //----------------------- DescDBListener stuff ---------------------------


    /** Monitor changes to the global descdb so we can maintain the 
     *  chunk-specific help menu.
     */
    public void addDesc (DescDBEvent e) { 
	EasyMenuBar menubar = ui_module.getEasyMenuBar();
	ChunkDesc d = e.getNewDesc();
        JMenuItem newmenu =
            menubar.addMenuItem ("Help/Chunk-Specific Help/" + d.getName());
	newmenu.addActionListener(this);

    }

    public void removeDesc (DescDBEvent e) {}
    public void replaceDesc (DescDBEvent e) {}
    public void removeAllDescs (DescDBEvent e) {}

}
