/***************** <Tweek heading BEGIN do not edit this line> ****************
 * Tweek
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 ***************** <Tweek heading END do not edit this line> *****************/

/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000, 2001 by Iowa State University
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
 *************** <auto-copyright.pl END do not edit this line> ***************/

package org.vrjuggler.tweek;

import java.io.File;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Vector;
import javax.swing.tree.DefaultMutableTreeNode;
import org.vrjuggler.tweek.beans.*;
import org.vrjuggler.tweek.beans.loader.BeanInstantiationException;
import org.vrjuggler.tweek.services.*;
import org.vrjuggler.tweek.net.corba.*;


/**
 * @since 1.0
 */
public class TweekCore
   implements BeanInstantiationListener
{
   // ========================================================================
   // Public methods.
   // ========================================================================

   public static TweekCore instance ()
   {
      if ( m_instance == null )
      {
         m_instance = new TweekCore();
      }

      return m_instance;
   }

   public void init (String[] args) throws Exception
   {
      // Register the internal static beans
      registerStaticBeans();

      // This needs to be the first step to ensure that all the basic services
      // and viewers get loaded.
      String defaultPath = System.getProperty("TWEEK_BASE_DIR")
                     + File.separator + "bin" + File.separator + "beans";

      try {
         findAndLoadBeans( defaultPath );
      }
      catch ( BeanPathException e )
      {
         System.out.println("WARNING: Invalid path " + defaultPath);
      }

      String[] new_args = parseTweekArgs(args);

      // Register the command-line arguments with the Environment Service (if
      // it is available).
      try
      {
         EnvironmentService service = (EnvironmentService)
            BeanRegistry.instance().getBean( EnvironmentService.class.getName() );

         if ( service != null )
         {
            service.setCommandLineArgs(new_args);
         }
      }
      catch (ClassCastException e)
      {
         System.err.println("WARNING: Failed to register command-line arguments");
      }

      m_gui = new TweekFrame();

      // Now we need to register the TweekFrame instance as a listener for
      // BeanFocusChangeEvents.
      List viewer_beans = BeanRegistry.instance().getBeansOfType( ViewerBean.class.getName() );
      for ( Iterator itr = viewer_beans.iterator(); itr.hasNext(); )
      {
         BeanModelViewer v = ((ViewerBean)itr.next()).getViewer();
         v.addBeanFocusChangeListener(m_gui);
      }

      m_gui.initGUI();
   }

   /**
    * Registers the beans that are internal to Tweek that are required to exist
    * before the bean loading can begin.
    */
   protected void registerStaticBeans()
   {
      BeanRegistry registry = BeanRegistry.instance();

      // environment service
      registry.registerBean( new EnvironmentService(
            new BeanAttributes( "Environment" ) ) );

      // global preferences service
      registry.registerBean( new GlobalPreferencesService(
            new BeanAttributes( "GlobalPreferences" ) ) );

   }

   /**
    * Called by the BeanInstantiationCommunicator singleton whenever a new bean
    * is instantiated.
    */
   public void beanInstantiated (BeanInstantiationEvent evt)
   {
      // If the bean created is a viewer bean, initialize it with tweek
      Object bean = evt.getBean();
      if ( bean instanceof ViewerBean ) {
         BeanModelViewer viewer = ((ViewerBean)bean).getViewer();
         viewer.setModel(panelTreeModel);
         viewer.initGUI();
      }
   }


   /**
    * Look for TweekBeans in standard locations and register them in the
    * registry.
    *
    * @param path    the path in which to search for TweekBeans
    */
   public void findAndLoadBeans( String path )
      throws BeanPathException
   {
      BeanRegistry registry = BeanRegistry.instance();

      // Get the beans in the given path
      XMLBeanFinder finder = new XMLBeanFinder();
      List beans = finder.find( path );
      for ( Iterator itr = beans.iterator(); itr.hasNext(); ) {
         TweekBean bean = (TweekBean)itr.next();

         // try to instantiate the bean
         try {
            bean.instantiate();
            registry.registerBean( bean );
         } catch( BeanInstantiationException e ) {
            System.err.println("WARNING: Failed to instantiate bean'" +
                               bean.getName() + "': " +
                               e.getMessage());
         }
      }
   }

   public BeanTreeModel getPanelTreeModel()
   {
      return panelTreeModel;
   }

   /**
    * Default constructor.
    */
   protected TweekCore ()
   {
      BeanInstantiationCommunicator.instance().addBeanInstantiationListener( this );
   }

   /**
    * Looks through the given array of arguments for any that are specific to
    * the Tweek Java GUI.  Those that are recognized are removed and handled
    * here.  Unrecognized options are left in the array.  The remaining
    * arguments are returned to the caller in a new array.
    *
    * @post Any Tweek-specific arguments are removed and a new array without
    *       those arguments is returned.
    */
   protected String[] parseTweekArgs (String[] args)
   {
      Vector save_args = new Vector();

      for ( int i = 0; i < args.length; i++ )
      {
         if ( args[i].startsWith("--beanpath=") )
         {
            int start = args[i].indexOf('=') + 1;
            String path = args[i].substring(start);
            try {
               findAndLoadBeans( path );
            }
            catch ( BeanPathException e )
            {
               System.out.println("WARNING: Invalid path " + path);
            }
         }
         else
         {
            save_args.add(args[i]);
         }
      }

      String[] new_args = null;

      if ( save_args.size() > 0 )
      {
         new_args = new String[save_args.size()];

         for ( int i = 0; i < save_args.size(); i++ )
         {
            new_args[i] = (String) save_args.elementAt(i);
         }
      }

      return new_args;
   }

   // ========================================================================
   // Protected data members.
   // ========================================================================

   protected static TweekCore m_instance = null;

   // ========================================================================
   // Private data members.
   // ========================================================================

   private TweekFrame m_gui = null;

   private BeanTreeModel panelTreeModel =
      new BeanTreeModel(new DefaultMutableTreeNode());
}
