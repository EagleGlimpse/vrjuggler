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

package org.vrjuggler.vrjconfig.commoneditors;

import java.awt.Color;
import java.awt.Font;
import java.awt.geom.Rectangle2D;
import java.util.Hashtable;
import java.util.Map;
import javax.swing.JComponent;

import org.jgraph.JGraph;
import org.jgraph.cellview.JGraphRoundRectView;
import org.jgraph.graph.AttributeMap;
import org.jgraph.graph.CellView;
import org.jgraph.graph.DefaultGraphModel;
import org.jgraph.graph.GraphConstants;
import org.jgraph.graph.GraphLayoutCache;
import org.jgraph.graph.GraphModel;
import org.jgraph.util.JGraphHeavyweightRedirector;
import org.jgraph.util.JGraphUtilities;

import org.vrjuggler.jccl.config.ConfigBroker;
import org.vrjuggler.jccl.config.ConfigBrokerProxy;
import org.vrjuggler.jccl.config.ConfigDefinitionRepository;

import org.vrjuggler.vrjconfig.commoneditors.devicegraph.*;


/**
 * An extension to the JGraph Swing component whose most useful feature is
 * setting up a special graph model, graph layout cache, and cell view factory
 * to help users avoid making very confusing mistakes.
 */
public class DeviceGraph
   extends JGraph
   implements EditorConstants
{
   /**
    * Initializes this JGraph specialization to use a model of type
    * <code>DeviceGraphModel</code>, a graph layout cache of type
    * <code>DeviceGraphLayoutCache</code>, and a cell view factory of type
    * <code>ProxiedDeviceCellViewFactory</code>.
    *
    * @see org.vrjuggler.vrjconfig.commoneditors.devicegraph.DeviceGraphModel
    * @see org.vrjuggler.vrjconfig.commoneditors.devicegraph.DeviceGraphLayoutCache
    * @see org.vrjuggler.vrjconfig.commoneditors.devicegraph.ProxiedDeviceCellViewFactory
    */
   public DeviceGraph()
   {
      super(new DeviceGraphModel());
      ProxiedDeviceCellViewFactory factory =
         new ProxiedDeviceCellViewFactory();

      ConfigBroker broker = new ConfigBrokerProxy();
      ConfigDefinitionRepository repos = broker.getRepository();

      factory.registerCreator(repos.get(SIM_DIGITAL_DEVICE_TYPE),
                              JGraphRoundRectView.class);
      factory.registerCreator(repos.get(SIM_POS_DEVICE_TYPE),
                              JGraphRoundRectView.class);
      factory.registerCreator(repos.get(DIGITAL_PROXY_TYPE),
                              ProxyVertexView.class);
      factory.registerCreator(repos.get(POSITION_PROXY_TYPE),
                              ProxyVertexView.class);

      setGraphLayoutCache(new DeviceGraphLayoutCache(getModel(), factory));

//      this.addMouseListener(new MouseDispatcher(this));
      this.addMouseListener(new JGraphHeavyweightRedirector(false, true));
   }

/*
   public DeviceGraph(GraphModel model)
   {
      super(model);
   }

   public DeviceGraph(GraphModel model, GraphLayoutCache cache)
   {
      super(model, cache);
   }
*/

   /**
    * Cause all cells to change their size to the cell's renderer's perferred
    * size. This will reveal any descriptions under the cell.
    *
    */
   public void autoSizeAll()
   {
      CellView[] views =
         this.getGraphLayoutCache().getMapping(
            JGraphUtilities.getVertices(getModel(),
                                        DefaultGraphModel.getAll(getModel()))
         );
      try
      {
         Hashtable table = new Hashtable();
         if (views != null)
         {
            for ( int i = 0; i < views.length; ++i )
            {
               CellView view = views[i];
               JComponent component =
                  (JComponent) view.getRendererComponent(this, false, false,
                                                         false);
               if ( component != null )
               {
                  this.add(component);
                  component.validate();
                  GraphHelpers.autoSizeCellView(component.getPreferredSize(),
                                                view, table);
               }
            }
         }
         this.getGraphLayoutCache().edit(table, null, null, null);
      }
      catch (Exception ee)
      {
         ee.printStackTrace();
      }
      this.repaint();
   }

   /**
    * Centralized creation of an attribute map for the line styles used for
    * edges connecting a proxy pointing at a device.
    */
   public static Map createProxyLineStyle()
   {
      AttributeMap map = new AttributeMap();
      GraphConstants.setAutoSize(map, true);
      GraphConstants.setLineEnd(map, GraphConstants.ARROW_TECHNICAL);
      GraphConstants.setEndSize(map, 10);
      GraphConstants.setDashPattern(map, new float[]{3, 0});
      return map;
   }

   /**
    * Centralized creation of an attribute map for graph cells that represent
    * the ConfigElement for a device.  Internally, this makes use of
    * <code>createDeviceAttributes(int,int,boolean)</code>.  It is invoked
    * with its <code>autoSize</code> parameter set to true.
    *
    * @param x  the x-coordinate for the device cell
    * @param y  the y-coordinate for the device cell
    *
    * @see #createDeviceAttributes(int,int,boolean)
    */
   public static Map createDeviceAttributes(int x, int y)
   {
      return createDeviceAttributes(x, y, true);
   }

   /**
    * Centralized creation of an attribute map for graph cells that represent
    * the ConfigElement for a device.  Internally, this makes use of
    * <code>JGraph.createBounds()</code>.
    *
    * @param x          the x-coordinate for the device cell
    * @param y          the y-coordinate for the device cell
    * @param autoSize   enable or disable always-on auto sizing of cells
    *
    * @see org.jgraph.graph.GraphConstants
    * @see org.jgraph.JGraph#createBounds(AttributeMap,int,int,Color)
    */
   public static Map createDeviceAttributes(int x, int y, boolean autoSize)
   {
      Map map = new AttributeMap();
      map = JGraph.createBounds((AttributeMap) map, x, y, Color.blue);

      GraphConstants.setAutoSize(map, autoSize);
      GraphConstants.setResize(map, true);

      return map;
   }

   /**
    * Centralized creation of an attribute map for graph cells that represent
    * the ConfigElement for a device proxy.  Internally, this makes use of
    * <code>createProxyAttributes(int,int,boolean)</code>.  It is invoked
    * with its <code>autoSize</code> parameter set to true.
    *
    * @param x  the x-coordinate for the proxy cell
    * @param y  the y-coordinate for the proxy cell
    *
    * @see #createProxyAttributes(int,int,boolean)
    */
   public static Map createProxyAttributes(int x, int y)
   {
      return createProxyAttributes(x, y, false);
   }

   /**
    * Centralized creation of an attribute map for graph cells that represent
    * the ConfigElement for a device proxy.  Internally, this makes use of
    * <code>JGraph.createBounds()</code>.
    *
    * @param x          the x-coordinate for the proxy cell
    * @param y          the y-coordinate for the proxy cell
    * @param autoSize   enable or disable always-on auto sizing of cells
    *
    * @see org.jgraph.graph.GraphConstants
    * @see org.jgraph.JGraph#createBounds(AttributeMap,int,int,Color)
    */
   public static Map createProxyAttributes(int x, int y, boolean autoSize)
   {
      Map map = new AttributeMap();
      map = JGraph.createBounds((AttributeMap) map, x, y, Color.red);

      GraphConstants.setAutoSize(map, autoSize);
      GraphConstants.setResize(map, true);

      return map;
   }

/*
   private static Map createBounds(AttributeMap map, int x, int y, Color c)
   {
      GraphConstants.setBounds(map, map.createRect(x, y, 90, 30));
      GraphConstants.setBorder(map, BorderFactory.createRaisedBevelBorder());
      GraphConstants.setBackground(map, c.darker().darker());
      GraphConstants.setGradientColor(map, c.brighter().brighter().brighter());
      GraphConstants.setForeground(map, Color.white);
   }
*/
}
