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




package VjControl;

import java.lang.String;
import java.lang.StringBuffer;
import java.util.Vector;

import VjConfig.*;
import VjControl.LogMessageListener;
import VjControl.LogMessageEvent;
import VjControl.CoreModule;
import VjControl.VjComponent;


/** Static owner-of-all-vjcontrol-objects.
 *  The primary job of the Core object is to own VjControl's core
 *  modules, such as the PerformanceModule or the UI.  Any component
 *  can call the Core's methods to get access to one of these core
 *  modules.  For example, the PerfAnalyzer GUI component requests
 *  access to the PerformanceModule, which stores the actual 
 *  performance data.
 *  <p>
 *  Core also contains a set of routines for outputting error and
 *  status messages (ConsoleErrorMessage etc.).  Core can print
 *  errors to stdout, and other components can register with Core
 *  to receive message notification.  This is used by ControlUI 
 *  to provide a status bar and by ConsolePane to record messages.
 * 
 *  @author Christopher Just
 *  @version $Revision$
 */
public class Core 
    implements ChunkDBListener,
               VjComponent {


    /** ComponentFactory which creates all VjControl components. */
    static public ComponentFactory component_factory;


    /** Global Descdb (used by VjConfig.ChunkFactory).
     *  This is kind of messy... this descdb is for vjcontrols' own 
     *  configuration, and initially it's the one used by ChunkFactory.
     *  But, when the ConfigModule is loaded, it creates its own "global"
     *  ChunkDescDB which subsumes this descdb's contents and 
     *  responsibilities.  This is mainly because a) it's easier to keep
     *  things like ControlUI updated from that one and b) there is no
     *  b.
     *  The ideal solution would be separate ChunkFactories for the GUI's
     *  own config files and the data files vjcontrol's supposed to edit.
     *  Barring creating two copies of the Config code in separate 
     *  packages, I can't actually do that.  <sigh>
     */
    static public ChunkDescDB descdb;
    static public ConfigChunkDB gui_chunkdb;
    static public ConfigChunkDB default_chunkdb;

    // private data

    static protected String component_name;

    /** Contains all CoreModules added to self */
    static private Vector modules;

    static private Vector pending_chunks;
    static private Vector registered_components;

    static private String[] command_line;

    /** Dummy instance used as source for CoreDBEvents. */
    static private Core instance;

    /* logmessage stuff */
    static private Vector logmessage_targets;
    static protected boolean no_logmessage_targets;
    static final private boolean info_msg_to_stdout = true;
    static final private boolean error_msg_to_stdout = true;

    /* this stuff should move into ui - deal with this when i muck with
     * prefs later this summer.
     */
    static public int    screenWidth;
    static public int    screenHeight;
    static public boolean window_pos_kludge;


    static public FileControl file;

    private Core() {
    }



    static public void initialize () {

	instance = new Core();
	
        component_name = "VjControl Core";

        file = new FileControl();

        component_factory = new ComponentFactory();
        modules = new Vector();
        pending_chunks = new Vector();
        registered_components = new Vector();

	window_pos_kludge = false;
	screenWidth = 800;
	screenHeight = 600;

	logmessage_targets = new Vector();
        no_logmessage_targets = true;

        command_line = new String[0];

  	descdb = new ChunkDescDB();
        descdb.setName ("VjControl default Global ChunkDescDB");
	default_chunkdb = new ConfigChunkDB();
	ChunkFactory.setDefaultChunkDB (default_chunkdb);
	ChunkFactory.setChunkDescDB (descdb);
	gui_chunkdb = new ConfigChunkDB ();

	gui_chunkdb.addChunkDBListener (instance);

        registerComponent (instance);

    }



    /** Gives a reference to the named module */
    static public CoreModule getModule (String name) {
        CoreModule c;
        for (int i = 0; i < modules.size(); i++) {
            c = (CoreModule)modules.elementAt (i);
            if (c.getComponentName().equalsIgnoreCase (name))
                return c;
        }
        return null;
    }



    /** Updates self based on VjControl global prefs */
    static protected void reconfigure(ConfigChunk ch) {
	// called whenever vjcontrol_preferences changes
	int i;
	String s;
	Property p;
	String defaultchunkfile = null;

	if (ch == null)
	    return;

	try {
	    //window_pos_kludge = ch.getPropertyFromToken("windowposkludge").getValue(0).getBool();

	    screenWidth = ch.getPropertyFromToken("windowsize").getValue(0).getInt();
	    screenHeight = ch.getPropertyFromToken("windowsize").getValue(1).getInt();	   

	}
	catch (Exception e) {
	    Core.consoleInfoMessage (component_name, "Old vjcontrol preferences file - please check preferences and re-save");
	}

	if (screenWidth < 1)
	    screenWidth = 800;
	if (screenHeight < 1)
	    screenHeight = 600;

    }



    //---------------------- Configuration Stuff ----------------------------

    // All of this code needs to be refactored somewhat to handle removing
    // as well as adding components.  in-place reconfiguration is also a 
    // good thing before the spec solidifies in juggler 1.2.0

    /** Adds an instantiation ConfigChunk to the queue of pending component
     *  instantiation chunks.  The ConfigChunk will be used to create an
     *  instance of a component as soon as its dependencies are met; this
     *  could happen immediately, or after the creation of some other 
     *  (dependency-satisfying) chunk.
     */
    static public void addPendingChunk (ConfigChunk ch) {
        pending_chunks.addElement (ch);
        //System.out.println ("addPendingChunk:\n" + ch.toString());
        for (;;) 
            if (!configProcessPending())
                break;
    }


    /** Processes the pending ConfigChunk list.  Util for addPendingChunk(). */
    static protected boolean configProcessPending () {
        boolean retval = false;
        for (int i = 0; i < pending_chunks.size(); i++) {
            ConfigChunk ch = (ConfigChunk)pending_chunks.elementAt(i);
            if (checkDependencies (ch)) {
                retval = addComponentConfig (ch);
                pending_chunks.removeElementAt(i);
                if (retval) 
                    Core.consoleInfoMessage (component_name, "Add Component: " 
                                             + ch.getName());
                else
                    Core.consoleErrorMessage (component_name, 
                                              "Add Component failed: " 
                                              + ch.getName());
                return true;
            }
        }
        return false;
    }


    /** Finds which component is ch's parent and calls addConfig for it.
     *  This shouldn't be confused with Core.addConfig, which just creates
     *  components that are children of Core directly.  addComponentConfig
     *  calls Core.addConfig when necessary.
     */
    static public boolean addComponentConfig (ConfigChunk ch) {
        try {
            Property p = ch.getPropertyFromToken ("parentcomp");
            String parentname = p.getValue(0).getString();
            VjComponent vjc = getComponentFromRegistry (parentname);
            if (vjc.addConfig (ch))
                return true;
            else {
                consoleErrorMessage (component_name, 
                                     "Creating component failed: " 
                                     + ch.getName());
                return false;
            }
        }
        catch (Exception e) {
            consoleErrorMessage (component_name, "Creating component failed: " 
                                 + ch.getName());
            e.printStackTrace();
            return false;
        }
    }



    /** true if dependencies are satisfied */
    static public boolean checkDependencies (ConfigChunk ch) {

        String cn = ch.getValueFromToken ("classname", 0).getString();
        if (!component_factory.isRegistered (cn))
            return false;

        Vector v = ch.getDependencyNames();
        for (int i = 0; i < v.size(); i++) {
            String s = (String)v.elementAt(i);
            if (getComponentFromRegistry(s) == null) {
                return false;
            }
        }
        return true;
    }


    static public void registerComponent (VjComponent vjc) {
        registered_components.addElement (vjc);
    }


    static public VjComponent getComponentFromRegistry (String name) {
        for (int i = 0; i < registered_components.size(); i++) {
            VjComponent vjc = (VjComponent)registered_components.elementAt(i);
            if (vjc.getComponentName().equalsIgnoreCase (name))
                return vjc;
        }
        return null;
    }


    //---------------------- Command-line argument handling -----------------

    static public void setCommandLine (String[] args) {
        command_line = args;
    }

    static public String[] getCommandLine () {
        return command_line;
    }


    //---------------------- Logging Routines -------------------------------

   
    /** Sends a "temporary" message to LogMessageListeners.
     *  Temporary messages should be put in status bars, etc., but do
     *  not generally have to be recorded in things like the main Messages
     *  console panel.  The individual listener is, of course, allowed to
     *  decide whether to display the message or not.
     */
    public static void consoleTempMessage (String s) {
	consoleTempMessage (null, s);
    }

    /** Sends a "temporary" message to LogMessageListeners.
     *  As consoleTempMessage (String s) but includes a source identifier.
     */
    public static void consoleTempMessage (String source, String s) {
	if (info_msg_to_stdout || no_logmessage_targets)
	    System.out.println (source + ": " + s);
	if (s == null || s.equals (""))
	    s = " ";

	notifyLogMessageTargets (new LogMessageEvent (instance, source, s, LogMessageEvent.TEMPORARY_MESSAGE));
    }


    public static void consoleInfoMessage (String s) {
	consoleInfoMessage ("", s);
    }
    public static void consoleInfoMessage (String source, String s) {
	if (info_msg_to_stdout || no_logmessage_targets)
	    System.out.println (source + ": " + s);
	notifyLogMessageTargets (new LogMessageEvent (instance, source, s, LogMessageEvent.PERMANENT_MESSAGE));
    }


    public static void consoleErrorMessage (String s) {
	consoleInfoMessage ("", s);
    }
    public static void consoleErrorMessage (String source, String s) {
	if (error_msg_to_stdout || no_logmessage_targets)
	    System.out.println ("Error (" + source + "): " + s);
	notifyLogMessageTargets (new LogMessageEvent (instance, source, s, LogMessageEvent.PERMANENT_ERROR));
    }



    //-------------------- LogMessage Target Stuff --------------------------

    static public synchronized void addLogMessageListener (LogMessageListener l) {
	synchronized (logmessage_targets) {
	    logmessage_targets.addElement (l);
	}
        no_logmessage_targets = false;
    }

    static public void removeLogMessageListener (LogMessageListener l) {
	synchronized (logmessage_targets) {
	    logmessage_targets.removeElement (l);
            if (logmessage_targets.size() == 0)
                no_logmessage_targets = true;
	}
    }

    static protected void notifyLogMessageTargets (LogMessageEvent e) {
	Vector l;
	synchronized (logmessage_targets) {
	    l = (Vector) logmessage_targets.clone();
	}
	for (int i = 0; i < l.size(); i++) {
	    LogMessageListener lis = (LogMessageListener)l.elementAt (i);
	    lis.logMessage (e);
	}
    }



    //-------------------- ChunkDBListener Stuff ----------------------------

    /** Core listens to ChunkDBEvents on the gui db for vjcontrol config */
    public void addChunk (ChunkDBEvent e) {
	reconfigure (e.getNewChunk());
    }
    public void removeChunk (ChunkDBEvent e) {;}
    public void replaceChunk (ChunkDBEvent e) {
	reconfigure (e.getNewChunk());
    }
    public void removeAllChunks (ChunkDBEvent e) {;}



    //-------------------- VjComponent Stuff --------------------------------

    public String getComponentName () {
        return component_name;
    }


    public boolean configure (ConfigChunk ch) {
        System.out.println ("Core doesn't really configure itself you putz." +
                            "Why is this even getting called?");
        return true;
    }


    public ConfigChunk getConfiguration () {
        return null;
    }


    public boolean addConfig (ConfigChunk ch) {
        try {
            String classname = ch.getValueFromToken ("classname", 0).getString();
            VjComponent vjc = component_factory.createComponent(classname);
            if (vjc.configure(ch)) {
                modules.addElement (vjc);
                registerComponent (vjc);
                return true;
            }
            else {
                return false;
            }
        }
        catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }


    public boolean removeConfig (String name) {
        return false;
    }


    /** This is a kludgey way of making sure that certain things get
     *  called at exit, such as ConfigSaveMonitorModule's checking.
     */
    public void destroy () {
        for (int i = 0; i < modules.size(); i++) {
            CoreModule cm = (CoreModule)modules.elementAt(i);
            cm.destroy();
        }
        System.exit(0);
    }

}


