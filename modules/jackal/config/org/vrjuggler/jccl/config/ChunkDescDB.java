/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2002 by Iowa State University
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
package org.vrjuggler.jccl.config;

import java.io.*;
import java.util.*;
import org.jdom.*;
import org.jdom.input.SAXBuilder;
import org.jdom.output.XMLOutputter;

/**
 * A container/database of ChunkDescs. This allows a set of ChunkDescs to be
 * grouped together logically.
 */
public class ChunkDescDB
{
   /**
    * Creates a new ChunkDesc database with initially no ChunkDescs contained
    * within it. The name defaults to "Unnamed".
    */
   public ChunkDescDB()
   {
   }

   /**
    * Sets the name of this ChunkDescDB.
    */
   public final void setName(String name)
   {
      this.name = name;
   }

   /**
    * Gets the name of this ChunkDescDB.
    */
   public final String getName()
   {
      return name;
   }

   public final void setInputFile(File file)
   {
      inputFile = file;
   }

   public final File getInputFile()
   {
      return inputFile;
   }

   /**
    * Builds the database using the given DOM node. If the node's type is
    * &lt;ChunkDescDB&gt;, the children are examined for ChunkDescs. ChunkDescs
    * found in the node's tree are added to this ChunkDescDB.
    *
    * @param elem    the DOM node from which to look for ChunkDescs
    *
    * @throws IOException  if there is an error while building the database
    */
   public void build(Element elem)
      throws IOException
   {
      loadChunkDescs(elem);
   }

   /**
    * Builds the database using the default input file. If the file contains an
    * XML document with the root type being &lt;ChunkDescDB&gt;, the children
    * are examined for ChunkDescs. ChunkDescs found in the node's tree are added
    * to this ChunkDescDB.
    *
    * @throws IOException  if there is an error while building the database
    */
   public void build()
      throws IOException
   {
      build(inputFile);
   }

   /**
    * Builds the database using the given file's contents. If the file contains
    * an XML document with the root type being &lt;ChunkDescDB&gt;, the children
    * are examined for ChunkDescs. ChunkDescs found in the node's tree are added
    * to this ChunkdescDB.
    *
    * @param file    the file from which to look for ChunkDescs
    *
    * @throws IOException  if there is an error while building the database
    */
   public void build(File file)
      throws IOException
   {
      if ( file != inputFile )
      {
         inputFile = file;
      }

      if ( null != inputFile || ! inputFile.canRead() )
      {
         // build the XML stream into a DOM tree
         try
         {
            SAXBuilder builder = new SAXBuilder();
            Document doc       = builder.build(file);

            loadChunkDescs(doc.getRootElement());
         }
         catch (JDOMException e)
         {
            throw new IOException(e.getMessage());
         }
      }
      else
      {
         throw new IOException("No readable file given for input!");
      }
   }

   /**
    * Builds the database using the given stream's contents. If the stream
    * contains an XML document with the root type being &lt;ChunkDescDB&gt;, the
    * children are examined for ChunkDescs. ChunkDescs found in the node's tree
    * are added to this ChunkdescDB.
    *
    * @param stream     the stream in which to look for ChunkDescs
    *
    * @throws IOException  if there is an error while building the database
    */
   public void build(InputStream stream)
      throws IOException
   {
      this.inputFile = null;

      // build the XML stream into a DOM tree
      try
      {
         SAXBuilder builder = new SAXBuilder();
         Document doc       = builder.build(stream);
         loadChunkDescs(doc.getRootElement());
      }
      catch (JDOMException e)
      {
         throw new IOException(e.getMessage());
      }
   }

   /**
    * Gets the number of ChunkDescs in this database.
    */
   public final synchronized int size()
   {
      return descs.size();
   }

   /**
    * Gets the nth ChunkDesc in this database.
    *
    * @param idx     the index of the ChunkDesc to retrieve
    */
   public final synchronized ChunkDesc get(int idx)
   {
      return (ChunkDesc)descs.get(idx);
   }

   public void fireDescDBUpdate (String origName, ChunkDesc updatedDesc)
   {
      notifyDescDBTargets(new DescDBEvent(this, DescDBEvent.REPLACE, origName,
                                          updatedDesc));
   }

   /**
    * Removes all ChunkDescs from this database, leaving this database empty.
    */
   public void clear()
   {
      synchronized(this)
      {
         descs.clear();
      }
      notifyDescDBTargets(new DescDBEvent(this, DescDBEvent.REMOVEALL, null));
   }

   /**
    * Removes the given ChunkDesc from this database. If the ChunkDesc is not in
    * this database, nothing is removed.
    *
    * @param desc    the ChunkDesc to remove
    *
    * @return  the ChunkDesc that was removed, null if nothing was removed
    */
   public ChunkDesc remove(ChunkDesc d)
   {
      return removeByToken(d.getToken());
   }

   /**
    * Removes the ChunkDesc from this database that has the given token. If the
    * ChunkDesc is not in this database, nothing is removed.
    *
    * @param tok     the token of the ChunkDesc to remove
    *
    * @return  the ChunkDesc that was removed, null if nothing was removed
    */
   public ChunkDesc removeByToken(String tok)
   {
      ChunkDesc d, retval = null;
      synchronized (this)
      {
         int n = descs.size();
         for (int i = 0; i < n; i++)
         {
            d = (ChunkDesc)descs.get(i);
            if (d.getToken().equals(tok))
            {
               retval = d;
               descs.remove(i);
               break;
            }
         }
      }
      if (retval != null)
      {
         DescDBEvent e = new DescDBEvent(this, DescDBEvent.REMOVE, retval);
         notifyDescDBTargets(e);
      }
      return retval;
   }

   /**
    * Removes the first ChunkDesc in this database that has the given name. If
    * no such ChunkDescs are in this database, nothing is removed.
    *
    * @param name    the name of the ChunkDesc to remove
    *
    * @return  the ChunkDesc that was removed, null if nothing was removed
    */
   public ChunkDesc removeByName(String name)
   {
      ChunkDesc d, retval = null;
      synchronized (this)
      {
         int n = descs.size();
         for (int i = 0; i < n; i++)
         {
            d = (ChunkDesc)descs.get(i);
            if ( d.getName().equals(name) )
            {
               retval = d;
               descs.remove(i);
               break;
            }
         }
      }
      if (retval != null)
      {
         DescDBEvent e = new DescDBEvent(this, DescDBEvent.REMOVE, retval);
         notifyDescDBTargets(e);
      }
      return retval;
   }

   /**
    * Retrieves a list of ChunkDescs whose tokens have the given prefix.
    *
    * @param prefix     the prefix value to test
    *
    * @return  the list of matching ChunkDescs. The list is empty if no
    *          ChunkDescs in this database have the prefix.
    */
   public List getTokenBegins(String prefix)
   {
      ChunkDesc d;
      ArrayList v = new ArrayList();
      ChunkDesc[] da;
      synchronized (this)
      {
         da = new ChunkDesc[descs.size()];
         da = (ChunkDesc[])descs.toArray(da);
      }
      for (int i = 0; i < da.length; i++)
      {
         //d = (ChunkDesc)descs.get(i);
         if ( da[i].getToken().startsWith(prefix) )
         {
            v.add(da[i]);
         }
      }
      return v;
   }

   /**
    * Gets the ChunkDesc in this database that has the given token.
    *
    * @param tok     the token of the ChunkDesc to retrieve
    *
    * @return  the matching ChunkDesc; null if nothing is found
    */
   public final ChunkDesc getByToken(String tok)
   {
      synchronized (this)
      {
         int n = descs.size();
         for (int i = 0; i < n; i++)
         {
            if (((ChunkDesc)descs.get(i)).getToken().equals(tok))
            {
               return (ChunkDesc)descs.get(i);
            }
         }
      }
      return null;
   }

   /**
    * Gets the first ChunkDesc in this database that has the given name.
    *
    * @param name    the name of the ChunkDesc to retrieve
    *
    * @return  the matching ChunkDesc; null if nothing is found
    */
   public final synchronized ChunkDesc getByName(String name)
   {
      int n = descs.size();
      for (int i = 0; i < n; i++)
      {
         if ( ((ChunkDesc)descs.get(i)).getName().equals(name) )
         {
            return (ChunkDesc)descs.get(i);
         }
      }
      return null;
   }

   public final synchronized String getTokenFromName(String _name)
   {
      int n = descs.size();
      for (int i = 0; i < n; i++)
      {
         if ( ((ChunkDesc)descs.get(i)).getName().equals(_name) )
         {
            return ((ChunkDesc)descs.get(i)).getToken();
         }
      }
      return null;
   }

   public final synchronized String getNameFromToken(String tok)
   {
      int n = descs.size();
      for (int i = 0; i < n; i++)
      {
         if ( ((ChunkDesc)descs.get(i)).getToken().equals(tok) )
         {
            return ((ChunkDesc)descs.get(i)).getName();
         }
      }
      return null;
   }

   public void setUnsavedChanges (boolean dirty)
   {
      mNeedToSave = dirty;
   }

   public boolean hasUnsavedChanges ()
   {
      return mNeedToSave;
   }

   /**
    * Adds the give chunk description object to this database's internal
    * collection of chunk descriptions.  A chunk description with the same
    * token will be removed before insertion.  The DOM node held by the chunk
    * description is added to this database's DOM tree.
    */
   public void add(ChunkDesc newDesc)
   {
      // Detach from the chunk description from its parent (if it has one).
      newDesc.getNode().detach();

      // Now do the insertion into the database.
      insertDesc(newDesc);
   }

   /**
    * Inserts the given chunk description into the internal database.  The
    * node held by d must already be part of the DOM tree.
    */
   private void insertDesc(ChunkDesc d)
   {
      // remove any desc with the same token
      removeByToken(d.getToken());

      synchronized (this)
      {
         // make sure the name will be unique
         if (getByName(d.getName()) != null)
         {
            // this is ugly & clunky, but it's a rarely occurring case,
            // and java (yay!) will handle the memory for us.  If we're
            // changing the name on insert, we need to allocate a new
            // chunkdesc with the new name, so that other chunkdescdbs
            // using this same chunk keep the original name.  since
            // we always replace instead of modifying chunkdescs, this
            // lets the same desc appear in 2 descdbs simultaneously.
            d = (ChunkDesc)d.clone();
            d.setName(generateUniqueName(d.getName()));
         }

         // do the deed
         descs.add(d);
      }

      DescDBEvent e = new DescDBEvent(this, DescDBEvent.INSERT, d);
      notifyDescDBTargets (e);
   }

   protected String generateUniqueName(String name)
   {
      String n;
      int i = 1;
      do
      {
         n = name + " <" + (i++) + ">";
      }
      while (getByName(n) != null);
      return n;
   }

   public void addAll(List v)
   {
      int n = v.size();
      for (int i = 0; i < n; i++)
      {
         add((ChunkDesc)v.get(i));
      }
   }

   public final void addAll(ChunkDescDB db)
   {
      synchronized (db)
      {
         addAll(db.descs);
      }
   }

   /**
    * Gets the XML representation of this ChunkDescDB as a String.
    */
   public String toString()
   {
      XMLOutputter outputter = new XMLOutputter("   ", true);
      outputter.setLineSeparator(System.getProperty("line.separator"));

      // Construct an XML document to write out.
      Document doc = new Document();
      this.makeDocument(doc);

      return outputter.outputString(doc);
   }

   public void write()
      throws IOException
   {
      write(inputFile);
   }

   public void write(File file)
      throws IOException
   {
      Document doc = new Document();
      this.makeDocument(doc);

      if ( inputFile != file )
      {
         inputFile = file;
      }

      XMLOutputter outputter = new XMLOutputter("  ", true);
      FileWriter writer      = new FileWriter(file);

      outputter.setLineSeparator(System.getProperty("line.separator"));
      outputter.output(doc, writer);
      writer.close();
   }

   public void write(OutputStream outStream)
      throws IOException
   {
      Document doc = new Document();
      this.makeDocument(doc);

      XMLOutputter outputter = new XMLOutputter("  ", true);
      outputter.setLineSeparator(System.getProperty("line.separator"));
      outputter.output(doc, outStream);
   }

   /*-******************* DescDB Target Stuff *********************/
   public synchronized void addDescDBListener(DescDBListener l)
   {
      synchronized (targets)
      {
         targets.add(l);
      }
   }

   public void removeDescDBListener(DescDBListener l)
   {
      synchronized (targets)
      {
         targets.remove(targets.indexOf(l));
      }
   }

   protected void notifyDescDBTargets(DescDBEvent e)
   {
      DescDBListener[] l = new DescDBListener[20];
      mNeedToSave = true;

      synchronized (targets)
      {
         l = (DescDBListener[]) targets.toArray(l);
      }

      int a = e.getAction();

      for (int i = 0; (i < l.length) && (l[i] != null); i++)
      {
         if (a == e.INSERT)
         {
            l[i].chunkDescAdded(e);
         }
         else if (a == e.REMOVE)
         {
            l[i].chunkDescRemoved(e);
         }
         else if (a == e.REPLACE)
         {
            l[i].chunkDescReplaced(e);
         }
         else if (a == e.REMOVEALL)
         {
            l[i].chunkDescsCleared(e);
         }
      }
   }

   protected void loadChunkDescs(Element docRoot)
      throws IOException
   {
      String name = docRoot.getName();

      if ( name.equals(ConfigTokens.chunk_desc_db_TOKEN) )
      {
         Iterator i = docRoot.getChildren().iterator();
         Element desc_elem;

         while ( i.hasNext() )
         {
            desc_elem = (Element) i.next();
            if ( desc_elem.getName().equals(ConfigTokens.chunk_desc_TOKEN) )
            {
               // We have to detach this node from its parent so that we can
               // create a new root document when writing out the database.
               this.insertDesc(new ChunkDesc((Element) desc_elem.clone()));
            }
            else
            {
            }
         }
      }
      else
      {
         throw new IOException("Unrecognized chunk description element '" +
                               name + "'.");
      }
   }

   private void makeDocument(Document doc)
   {
      Element root = new Element(ConfigTokens.chunk_desc_db_TOKEN);
      doc.setRootElement(root);

      ChunkDesc cur_desc;

      Iterator i = descs.iterator();
      while ( i.hasNext() )
      {
         cur_desc = (ChunkDesc) i.next();

         // Ensure that the node isn't attached somewhere else.
         cur_desc.getNode().detach();
         root.addContent(cur_desc.getNode());
      }
   }

   /**
    * The name of this ChunkDescDB.
    */
   private String name = "Unnamed";

   private File inputFile = null;

   private List targets = new ArrayList();
   private List descs   = new ArrayList();

   private boolean mNeedToSave = false;
}
