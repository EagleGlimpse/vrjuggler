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


package VjComponents.ConfigEditor.ConfigChunkUI;

import java.awt.*;
import java.awt.event.*;
import java.util.Vector;
import javax.swing.*;
import javax.swing.border.*;

import VjConfig.*;
import VjComponents.ConfigEditor.*;
import VjComponents.UI.Widgets.*;
import VjControl.Core;


/** Default implementation of ConfigChunkPanel.
 *  DO NOT INSTANTIATE THIS YOURSELF.  ConfigChunkPanels should be allocated
 *  via ConfigUIHelperModule (and its ConfigChunkPanelFactory).
 */
public class DefaultConfigChunkPanel extends JPanel 
    implements ConfigChunkPanel, ChunkDBListener { 

    ConfigChunk   chunk;
    ConfigChunkDB chunkdb;
    Vector proppanels; // property description panels.
    JPanel properties;
    JButton cancelbutton;
    JButton okbutton;
    JButton applybutton;
    JButton helpbutton;
    JTextField namef;
    JTextField helpfield;
    JScrollPane sp;

    GridBagLayout playout;
    GridBagConstraints pconstraints;

    protected ConfigChunk component_chunk;


    public DefaultConfigChunkPanel () {
        super();

        //component_name = "Unconfigured DefaultConfigChunkPanel";
        component_chunk = null;

	JPanel northpanel, centerpanel;

	proppanels = new Vector();

	chunk = null;
	chunkdb = null;
	//setBorder (new EmptyBorder (5, 5, 0, 5));
	setLayout (new BorderLayout (5,5));

	JPanel p1;
	northpanel = new JPanel();
	northpanel.setLayout (new BoxLayout (northpanel, BoxLayout.Y_AXIS));
	p1 = new JPanel();
	p1.setLayout (new BoxLayout (p1, BoxLayout.X_AXIS));
	p1.add (new JLabel ("Instance Name: "));
	p1.add (namef = new StringTextField ("", 25));
	northpanel.add (p1);

        helpfield = new JTextField ("");
        helpfield.setEditable (false);
        northpanel.add (helpfield);

	add(northpanel, "North");
	
	properties = new JPanel ();
	properties.setLayout (new BoxLayout (properties, BoxLayout.Y_AXIS));

	sp = new JScrollPane(properties, JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
			     JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
	sp.getVerticalScrollBar().setUnitIncrement(40);
	sp.getHorizontalScrollBar().setUnitIncrement(40);

	add(sp,"Center");
	

    }



    /* note: _chunkdb is specifically allowed to be null; this is often
     * the case w/ embedded chunks.  if chunkdb is non-null we'll listen
     * to it for changes to _chunk.  otherwise, whoever called setChunk
     * on us is responsible for keeping us up-to-date.
     */
    public void setChunk (ConfigChunk _chunk, ConfigChunkDB _chunkdb) {
        if (chunk == _chunk)
            return;

	chunk = _chunk;
	chunkdb = _chunkdb;

        System.out.println ("editing chunk: " + _chunk.toString());

        // double check this to make sure i'm not leaving anything
        // dangling.
        proppanels.removeAllElements();
        properties.removeAll();

        if (chunk != null) {
            namef.setText (chunk.getLastNameComponent());
            helpfield.setText (chunk.desc.help);

            // make property panels
            PropertyPanel t;
            for (int i = 0; i < chunk.props.size(); i++) {
                t = new PropertyPanel ((Property)chunk.props.elementAt(i), 
                                       this);
                proppanels.addElement(t);
                properties.add(t);
            }
        }

        if (chunkdb != null)
            chunkdb.addChunkDBListener (this);
    }


    public String getName () {
	return chunk.getName();
    }


    public String getComponentName () {
        return getName();
    }


    public boolean configure (ConfigChunk ch) {
        component_chunk = ch;
        //component_name = ch.getValueFromToken("Name", 0).getString();
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
        if (chunkdb != null)
            chunkdb.removeChunkDBListener (this);
    }


    public ConfigChunkDB getChunkDB () {
	return chunkdb;
    }


    public ConfigChunk getOldValue() {
	return chunk;
    }



    public ConfigChunk getNewValue() {
	/* returns a configchunk based on the values current 
	 * in this window */
        String name = chunk.getName();
        name = ConfigChunk.setLastNameComponent (name, namef.getText());
	ConfigChunk c = ChunkFactory.createChunk (chunk.desc, name);
	c.props.removeAllElements();
	for (int i = 0; i < chunk.props.size(); i++) {
	    c.props.addElement (((PropertyPanel)proppanels.elementAt(i)).getValue());
	}
        c.validateEmbeddedChunkNames();
	return c;
    }



    public boolean matches (String cl, Object db, Object o) {
        try {
            if (cl != null) {
                if (!(Class.forName(cl).isInstance(this)))
                    return false;
            }
            if (chunkdb != db)
                return false;
            ConfigChunk ch = (ConfigChunk)o;
            return (ch == null) || (ch.getName().equals(chunk.getName()));
        }
        catch (Exception e) {
            return false;
        }
    }


    public void showHelp () {
        ConfigUIHelper uihelper_module = (ConfigUIHelper)Core.getModule ("ConfigUIHelper Module");
        if (uihelper_module != null) 
            uihelper_module.loadDescHelp (chunk.desc.getToken());
        else
            Core.consoleErrorMessage ("DefaultConfigChunkPanel",
                                      "Couldn't load help: ConfigUI Helper doesn't exist.");
    }


    //------------------- ChunkDBListener stuff ----------------------
    public void addChunk (ChunkDBEvent e) {;}
    public void removeChunk (ChunkDBEvent e) {;}
    public void replaceChunk (ChunkDBEvent e) {
        // basically we're checking for when an apply comes across, so we
        // can correct exactly which chunk we're supposed to be editing.
        // at some point, we should add the ability to completely update the
        // displayed state; that'll be helpful when we get more "live" 
        // information from juggler.  (it'd probably be helpful at that
        // point if we can query the ConfigModule about whether this is an
        // "active db" or not - if not, then we shouldn't have to do a full
        // update).
        if (e.getOldChunk() == chunk)
            chunk = e.getNewChunk();
    }
    public void removeAllChunks (ChunkDBEvent e) {;}


}




