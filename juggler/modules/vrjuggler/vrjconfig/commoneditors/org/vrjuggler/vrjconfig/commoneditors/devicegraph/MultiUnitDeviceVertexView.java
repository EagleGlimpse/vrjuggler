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

package org.vrjuggler.vrjconfig.commoneditors.devicegraph;

import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Frame;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.*;
import javax.swing.*;

import org.jgraph.JGraph;
import org.jgraph.graph.*;

import info.clearthought.layout.*;

import org.vrjuggler.jccl.config.ConfigBroker;
import org.vrjuggler.jccl.config.ConfigBrokerProxy;
import org.vrjuggler.jccl.config.ConfigContext;
import org.vrjuggler.jccl.config.ConfigElement;
import org.vrjuggler.jccl.config.event.ConfigElementEvent;
import org.vrjuggler.jccl.config.event.ConfigElementListener;

import org.vrjuggler.vrjconfig.commoneditors.EditorConstants;


/**
 * A custom vertex view for graph cells representing device config elements
 * that have multiple input sources (which are in turn represented with
 * JGraph ports).  This view makes use of heavy-weight renderers so that users
 * may interact with components in the vertex view.
 */
public class MultiUnitDeviceVertexView
   extends VertexView
{
   private transient DeviceVertexRenderer renderer =
      new DeviceVertexRenderer();

   public MultiUnitDeviceVertexView()
   {
      super();
   }

   public MultiUnitDeviceVertexView(Object cell)
   {
      super(cell);
   }

   public CellViewRenderer getRenderer()
   {
      return renderer;
   }

   private static class DeviceVertexRenderer
      extends AbstractCustomVertexRenderer
      implements ConfigElementListener
   {
      private static final int PORT_START_COLUMN    = 1;
      private static final int PORT_END_COLUMN      = 1;
      private static final int LABEL_START_COLUMN   = 3;
      private static final int LABEL_END_COLUMN     = 3;
      private static final int BUTTON0_START_COLUMN = 5;
      private static final int BUTTON0_END_COLUMN   = 5;

      private static final int SPAN_START_COLUMN = PORT_START_COLUMN;
      private static final int SPAN_END_COLUMN   = BUTTON0_END_COLUMN;

      private static final int BUTTON_MIN_WIDTH   = 30;
      private static final int BUTTON_MIN_HEIGHT  = 16;
      private static final int BUTTON_PREF_WIDTH  = 30;
      private static final int BUTTON_PREF_HEIGHT = 16;

      private static final int NAME_ROW = 1;

      private static String IMG_ROOT = EditorConstants.COMMON_IMG_ROOT;

      public DeviceVertexRenderer()
      {
         super();

         double[][] sizes =
            {
               {5, TableLayout.MINIMUM, 5, TableLayout.FILL, 5,
                TableLayout.MINIMUM, 5},
               {5, TableLayout.PREFERRED}
            };
         mMainLayout = new TableLayout(sizes);
         this.setLayout(mMainLayout);

         nameLabel.setText(" ");

         this.add(nameLabel,
                  new TableLayoutConstraints(SPAN_START_COLUMN, NAME_ROW,
                                             SPAN_END_COLUMN, NAME_ROW,
                                             TableLayoutConstraints.FULL,
                                             TableLayoutConstraints.CENTER));
      }

      public Component getRendererComponent(JGraph graph, CellView view,
                                            boolean selected, boolean hasFocus,
                                            boolean preview)
      {
         mView = view;

         this.graph    = graph;
         this.selected = selected;
         this.preview  = preview;
         this.hasFocus = hasFocus;

         Map attributes = view.getAllAttributes();
         installAttributes(graph, attributes);

         if ( mDeviceInfo == null )
         {
            try
            {
               DefaultGraphCell cell = (DefaultGraphCell) view.getCell();
               mDeviceInfo = (DeviceInfo) cell.getUserObject();

               mVariableUnits = mDeviceInfo.hasVariableUnitCount();

               Dimension label_size = nameLabel.getPreferredSize();

               int min_width = label_size.width;

               // The extra 10 units of height is to allow some vertical
               // breathing room.
               int min_height = label_size.height + 10;

               setMinimumSize(new Dimension(min_width, min_height));
               setPreferredSize(new Dimension(min_width, min_height));

               ImageIcon add_icon = null;
               try
               {
                  ClassLoader loader = getClass().getClassLoader();
                  add_icon =
                     new ImageIcon(loader.getResource(IMG_ROOT + "/New16.gif"));
               }
               catch (Exception ex)
               {
               }

               int extra_width = min_width, extra_height = 0;

               for ( Iterator i = mDeviceInfo.getUnitTypes().iterator();
                     i.hasNext(); )
               {
                  final Integer unit_type = (Integer) i.next();

                  if ( mVariableUnits )
                  {
                     JButton add_button = new JButton();
                     add_button.setEnabled(true);
                     add_button.setToolTipText(
                        "Add a new input source for this device"
                     );

                     add_button.setText("Create " +
                                        getUnitTypeName(unit_type) +
                                        " Input Source");
                     add_button.setMargin(new Insets(3, 0, 0, 0));
                     add_button.setBorderPainted(false);
                     add_button.setFont(new Font("Dialog", Font.PLAIN, 10));
                     add_button.setContentAreaFilled(false);
                     add_button.setIcon(add_icon);

                     add_button.addActionListener(
                        new ActionListener()
                        {
                           public void actionPerformed(ActionEvent evt)
                           {
                              // This adds a new property value to our config
                              // element which results in propertyValueAdded()
                              // being invoked.  It is there that we add the
                              // new row to the layout.
                              addUnit(unit_type);
                           }
                        }
                     );

                     UnitTypeGroup ug = new UnitTypeGroup(add_button, 0);
                     mUnitGroups.put(unit_type, ug);
                     int button_row = mMainLayout.getNumRow();
                     mMainLayout.insertRow(button_row, TableLayout.MINIMUM);

                     this.add(add_button,
                              new TableLayoutConstraints(SPAN_START_COLUMN,
                                                         button_row,
                                                         SPAN_END_COLUMN,
                                                         button_row,
                                                         TableLayoutConstraints.CENTER,
                                                         TableLayoutConstraints.CENTER));

                     Dimension add_size = add_button.getPreferredSize();
                     extra_width   = Math.max(extra_width, add_size.width);
                     extra_height += add_size.height;
                  }

                  for ( Iterator c = cell.getChildren().iterator();
                        c.hasNext(); )
                  {
                     Object child = c.next();

                     if ( child instanceof DefaultPort )
                     {
                        DefaultPort port = (DefaultPort) child;
                        UnitInfo unit_info = (UnitInfo) port.getUserObject();
                        if ( unit_info.getUnitType().equals(unit_type) )
                        {
                           addUnitRow((DefaultPort) child, false);
                        }
                     }
                  }
               }

               Dimension pref_size = getPreferredSize();
               setPreferredSize(
                  new Dimension(Math.max(pref_size.width, extra_width),
                                pref_size.height + extra_height)
               );

               // At this point, we only revalidate the layout.  There is no
               // need to change the cell view size because it is still being
               // set up by JGraph at this stage of execution.
               this.revalidate();

               // Avoid adding ourselves as a listener multiple times.
               // XXX: How can this happen?  The test for mDeviceInfo being
               // null above should be sufficient to guarantee that this code
               // only gets executed once per instance of this type...
               if ( ! preview )
               {
                  mDeviceInfo.getElement().addConfigElementListener(this);
               }
            }
            catch (Exception ex)
            {
               ex.printStackTrace();
            }
         }

         if ( graph.getEditingCell() != view.getCell() )
         {
            nameLabel.setText(mDeviceInfo.getElement().getName());
         }
         else
         {
            nameLabel.setText(" ");
         }

         return this;
      }

      public void nameChanged(ConfigElementEvent evt)
      {
         nameLabel.setText(mDeviceInfo.getElement().getName());
      }

      public void propertyValueAdded(ConfigElementEvent evt)
      {
         if ( mVariableUnits )
         {
            Collection unit_types = mDeviceInfo.getUnitTypes();

            for ( Iterator t = unit_types.iterator(); t.hasNext(); )
            {
               Integer type = (Integer) t.next();
               String prop_token = mDeviceInfo.getUnitPropertyToken(type);

               // If the device has a variable unit count and the value added
               // was for the device unit property, then we need to add another
               // row to the renderer to repersent the new unit.
               if ( evt.getProperty().equals(prop_token) )
               {
                  int unit_num = mDeviceInfo.getUnitCount(type) - 1;
                  DefaultPort port = GraphHelpers.createDevicePort(type,
                                                                   unit_num);
                  ((DefaultGraphCell) mView.getCell()).add(port);
                  addUnitRow(port, true);

                  break;
               }
            }
         }
      }

      public void propertyValueChanged(ConfigElementEvent evt)
      {
         if ( mVariableUnits )
         {
            Collection unit_types = mDeviceInfo.getUnitTypes();

            for ( Iterator t = unit_types.iterator(); t.hasNext(); )
            {
               Integer type = (Integer) t.next();
               String prop_token = mDeviceInfo.getUnitPropertyToken(type);

               // If the device has a variable unit count and the value added
               // was for the device unit property, then we need to add another
               // row to the renderer to repersent the new unit.
               if ( evt.getProperty().equals(prop_token) )
               {
                  int cur_count = mDeviceInfo.getUnitCount(type);
                  int old_count = ((Number) evt.getValue()).intValue();

                  System.out.println("cur_count == " + cur_count);
                  System.out.println("old_count == " + old_count);

                  // Unit addition.  This always appends the new unit.
                  if ( cur_count > old_count )
                  {
                     System.out.println("Adding new row");
                     UnitInfo new_unit_info =
                        new UnitInfo(type, new Integer(cur_count - 1));
                     DefaultPort port =
                        GraphHelpers.createDevicePort(new_unit_info);

                     ((DefaultGraphCell) mView.getCell()).add(port);
                     addUnitRow(port, true);

                     // Keep track of the rows that are added dynamically.
                     // These may need to be removed later as a result of an
                     // undo operation.
                     mAddedUnitRows.push(new_unit_info);
                  }
                  // Unit removal.
                  else
                  {
                     System.out.println("Removing a row");
                     DefaultPort port = null;

                     // This bit is needed for handling undo and redo
                     // correctly.  mRemovedUnitInfo is only non-null when
                     // a row is removed as a result of the user clicking on
                     // the "Delete" button for a row.
                     if ( mRemovedUnitInfo == null )
                     {
                        if ( ! mAddedUnitRows.empty() )
                        {
                           UnitInfo row_unit_info =
                              (UnitInfo) mAddedUnitRows.pop();
                           port = findPort((DefaultGraphCell) mView.getCell(),
                                           row_unit_info);
                        }
                     }
                     else
                     {
                        mAddedUnitRows.remove(mRemovedUnitInfo);
                        port = findPort((DefaultGraphCell) mView.getCell(),
                                        mRemovedUnitInfo);
                     }

                     if ( port != null )
                     {
                        removeUnitRow(port);
                        mRemovedUnitInfo = null;
                     }
                     else
                     {
                        System.err.println("WARNING: Could not find port " +
                                           "to remove containing an object " +
                                           "matching " + mRemovedUnitInfo);
                     }
                  }

                  break;
               }
            }
         }
      }

      public void propertyValueRemoved(ConfigElementEvent evt)
      {
         // NOTE: This method does not make use of the hack member variable
         // mRemovedUnitInfo.  The removed unit can be determined through a
         // searching process.  This method could be sped up slightly for
         // the case of a device supporting multiple unit types, but the
         // search through the vertex's children is still the slow part either
         // way.  -PH 3/22/2005

         if ( mVariableUnits )
         {
            Collection unit_types = mDeviceInfo.getUnitTypes();

            for ( Iterator t = unit_types.iterator(); t.hasNext(); )
            {
               Integer type = (Integer) t.next();
               String prop_token = mDeviceInfo.getUnitPropertyToken(type);

               if ( evt.getProperty().equals(prop_token) )
               {
                  ConfigElement elt = mDeviceInfo.getElement();
                  int old_unit = evt.getIndex();
                  UnitInfo old_info = new UnitInfo(type, new Integer(old_unit));

                  DefaultPort port =
                     findPort((DefaultGraphCell) mView.getCell(), old_info);

                  if ( port != null )
                  {
                     removeUnitRow(port);

                     // Though we did not make use of this variable here, we
                     // should still reset it to be safe.
                     mRemovedUnitInfo = null;
                  }
                  else
                  {
                     System.err.println("WARNING: Could not find port to " +
                                        "remove containing an object " +
                                        "matching " + old_info);
                  }

                  break;
               }
            }
         }
      }

      private DefaultPort findPort(DefaultGraphCell parent,
                                   UnitInfo searchUnitInfo)
      {
         List children = parent.getChildren();
         DefaultPort port = null;

         for ( Iterator c = children.iterator(); c.hasNext(); )
         {
            Object child = c.next();
            if ( child instanceof DefaultPort )
            {
               UnitInfo cur_unit_info =
                  (UnitInfo) ((DefaultPort) child).getUserObject();
               System.out.println("[findPort()] Comparing " + searchUnitInfo +
                                  " with " + cur_unit_info);
               System.out.println("             Result == " +
                                  searchUnitInfo.equals(cur_unit_info));
               if ( searchUnitInfo.equals(cur_unit_info) )
               {
                  port = (DefaultPort) child;
                  break;
               }
            }
         }

         return port;
      }

      private String getUnitTypeName(Integer unitType)
      {
         String type_str = "Unknown";

         if ( unitType.equals(UnitConstants.ANALOG) )
         {
            type_str = "Analog";
         }
         else if ( unitType.equals(UnitConstants.COMMAND) )
         {
            type_str = "Command";
         }
         else if ( unitType.equals(UnitConstants.DIGITAL) )
         {
            type_str = "Digital";
         }
         else if ( unitType.equals(UnitConstants.GLOVE) )
         {
            type_str = "Glove";
         }
         else if ( unitType.equals(UnitConstants.KEYBOARD_MOUSE) )
         {
            type_str = "Keyboard/Mouse";
         }
         else if ( unitType.equals(UnitConstants.POSITION) )
         {
            type_str = "Position";
         }
         else if ( unitType.equals(UnitConstants.STRING) )
         {
            type_str = "String";
         }

         return type_str;
      }

      /**
       * Adds a new row for the given port that represents the input source
       * (device unit) for the input device represented by our vertex.
       * If the caller chooses not to resize the cell view after adding the
       * new row to this renderer, it is the responsibility of the caller to
       * perform the layout validation and the cell view resizing.  The
       * attributes of the port will be changed so that its bounds place it
       * within this row of the renderer layout.
       *
       * @param unitPort        the port for the input source represented by
       *                        the newly added row
       * @param resizeCellView  a flag indicating whether or not to revalidate
       *                        the renderer layout and to resize the cell
       *                        view associated with this renderer
       *
       * @see #revalidate()
       * @see GraphHelpers#autoSizeCellView(JGraph,Dimension,CellView)
       */
      private void addUnitRow(DefaultPort unitPort, boolean resizeCellView)
      {
         UnitInfo unit_info = (UnitInfo) unitPort.getUserObject();
         final Integer unit_type = unit_info.getUnitType();

         PortComponent port_widget = new PortComponent(this.graph, unitPort);
         port_widget.setMinimumSize(new Dimension(5, 5));
         port_widget.setPreferredSize(new Dimension(5, 5));
         port_widget.setForeground(nameLabel.getForeground());

         JLabel name_field = new JLabel();
         setLabelText(name_field, unit_info);
         name_field.setFont(new Font("Dialog", Font.ITALIC,
                                     nameLabel.getFont().getSize()));
         name_field.setBorder(BorderFactory.createLineBorder(Color.black));
         name_field.setBackground(
            gradientColor != null ? gradientColor.brighter().brighter()
                                  : this.getBackground().brighter().brighter()
         );

         int row, label_start_col, label_end_col;

         // If we have variable units, we have to place the new unit relative
         // to its "Add" button.
         if ( mVariableUnits )
         {
            UnitTypeGroup ug = (UnitTypeGroup) mUnitGroups.get(unit_type);

            // Account for the addition of the new unit to this group.
            ug.unitCount++;

            // Calculate the new row as an offset to the "Add" button.
            TableLayoutConstraints c = mMainLayout.getConstraints(ug.addButton);
            row = c.row1 + ug.unitCount;

            label_start_col = LABEL_START_COLUMN;
            label_end_col   = LABEL_END_COLUMN;
         }
         // If we have static units, then they are being added as a result of
         // the initialization of this renderer.  As such, they are being
         // added sequentially, so we can just tack on another row.
         else
         {
            row             = mMainLayout.getNumRow();
            label_start_col = LABEL_START_COLUMN;
            label_end_col   = BUTTON0_END_COLUMN;
         }

         mMainLayout.insertRow(row, TableLayoutConstraints.PREFERRED);
         this.add(port_widget,
                  new TableLayoutConstraints(PORT_START_COLUMN, row,
                                             PORT_END_COLUMN, row,
                                             TableLayoutConstraints.CENTER,
                                             TableLayoutConstraints.CENTER));
         this.add(name_field,
                  new TableLayoutConstraints(label_start_col, row,
                                             label_end_col, row,
                                             TableLayoutConstraints.FULL,
                                             TableLayoutConstraints.CENTER));

         int button_width = 0;

         if ( mVariableUnits )
         {
            JButton remove_btn = new JButton();
            remove_btn.setEnabled(true);
            remove_btn.setToolTipText("Delete this input source");

            try
            {
               ClassLoader loader = getClass().getClassLoader();

               remove_btn.setIcon(
                  new ImageIcon(loader.getResource(IMG_ROOT + "/Delete16.gif"))
               );
               remove_btn.setMargin(new Insets(0, 0, 0, 0));
               remove_btn.setBorderPainted(false);
               remove_btn.setFocusPainted(false);
               remove_btn.setContentAreaFilled(false);
            }
            catch (Exception ex)
            {
               remove_btn.setMinimumSize(new Dimension(BUTTON_MIN_WIDTH,
                                                       BUTTON_MIN_HEIGHT));
               remove_btn.setPreferredSize(new Dimension(BUTTON_PREF_WIDTH,
                                                         BUTTON_PREF_HEIGHT));
               remove_btn.setFont(new Font("Dialog", Font.PLAIN, 9));
               remove_btn.setText("Delete");
            }

            remove_btn.addActionListener(
               new ActionListener()
               {
                  public void actionPerformed(ActionEvent evt)
                  {
                     JComponent button = (JComponent) evt.getSource();
                     UnitTypeGroup ug =
                        (UnitTypeGroup) mUnitGroups.get(unit_type);
                     TableLayoutConstraints my_tlc =
                        mMainLayout.getConstraints(button);
                     TableLayoutConstraints add_btn_tlc =
                        mMainLayout.getConstraints(ug.addButton);

                     // This changes a new property value in our config
                     // element which results in a ConfigElementEvent being
                     // fired.  It is in the handling of that event that we
                     // remove the row from the layout.
                     removeUnit(my_tlc.row1 - add_btn_tlc.row1 - 1, unit_type);
                  }
               }
            );

            List components = new ArrayList(3);
            components.add(0, port_widget);
            components.add(1, name_field);
            components.add(2, remove_btn);
            mUnitRowMap.put(unit_info, components);

            this.add(remove_btn,
                     new TableLayoutConstraints(BUTTON0_START_COLUMN, row,
                                                BUTTON0_END_COLUMN, row,
                                                TableLayoutConstraints.CENTER,
                                                TableLayoutConstraints.CENTER));

            button_width = remove_btn.getPreferredSize().width;
         }

         // Update the preferred size of the renderer based on the
         // dimensions of the newly added row.
         int width  = port_widget.getPreferredSize().width +
                         name_field.getPreferredSize().width + button_width;
         int height = name_field.getPreferredSize().height;

         Dimension pref_size = this.getPreferredSize();
         int new_width  = Math.max(width, pref_size.width);
         int new_height = height + pref_size.height;
         pref_size = new Dimension(new_width, new_height);
         this.setPreferredSize(pref_size);

         // Only resize the cell view associated with this renderer if we are
         // told to do so.  This is a bit of an optimization.
         if ( resizeCellView )
         {
            this.revalidate();
            GraphHelpers.autoSizeCellView(this.graph, pref_size, mView);
         }
      }

      private void setLabelText(JLabel label, UnitInfo unitInfo)
      {
         label.setText(getUnitTypeName(unitInfo.getUnitType()) + " Unit: " +
                       unitInfo.getUnitNumber());
      }

      /**
       * Handles the addition of a new unit to the device by the user.  This
       * can only happen by clicking the button for adding a new unit,
       * which implies that this device has a variable unit count.
       */
      private void addUnit(Integer unitType)
      {
         mDeviceInfo.addUnit(unitType);
      }

      private void removeUnitRow(DefaultPort port)
      {
         List components = (List) mUnitRowMap.get(port.getUserObject());
         removeUnitRow(port, components);
         mUnitRowMap.remove(port.getUserObject());
      }

      /**
       *
       */
      private void removeUnitRow(DefaultPort port, List components)
      {
         // Get the preferred size of the renderer so that we can modify it
         // after the row is removed.
         Dimension pref_size = this.getPreferredSize();

         // Set up the new preferred width and height based on the row that
         // is being removed.
         int width = pref_size.width;
         int height = pref_size.height;
         int max_height = 0;

         // Find the row containing the components being removed.  We expect
         // that all the components in the given array are in the same row.
         // XXX: Is there a more robust way to do this?
         TableLayoutConstraints tlc =
            mMainLayout.getConstraints((JComponent) components.get(0));
         int row = tlc.row1;

         for ( Iterator c = components.iterator(); c.hasNext(); )
         {
            JComponent component = (JComponent) c.next();

            if ( max_height < component.getPreferredSize().height )
            {
               max_height = component.getPreferredSize().height;
            }

            this.remove(component);
         }

         // The height of the renderer will be decreased by the component
         // with the largest height in the given array of components.
         height -= max_height;

         mMainLayout.deleteRow(row);
         List removed_edges    = new ArrayList();
         List removed_vertices = new ArrayList();

         Frame parent = (Frame) SwingUtilities.getAncestorOfClass(Frame.class,
                                                                  this);

         // For each of the edges associated with port (port will always be a
         // target), update the proxy on the other end of te edge (the source).
         // The proxy will be updated to refer to no device since the unit it
         // was expecting has gone away.
         for ( Iterator e = this.graph.getModel().edges(port); e.hasNext(); )
         {
            Edge edge = (Edge) e.next();
            removed_edges.add(edge);

            DefaultPort proxy_port = (DefaultPort) edge.getSource();
            DefaultGraphCell proxy_cell =
               (DefaultGraphCell) proxy_port.getParent();
            ProxyInfo proxy_info = (ProxyInfo) proxy_cell.getUserObject();
            ConfigElement proxy_elt = proxy_info.getElement();
            proxy_elt.setProperty(EditorConstants.DEVICE_PROPERTY, 0, "",
                                  proxy_info.getContext());

            int option =
               JOptionPane.showConfirmDialog(
                  parent,
                  "Remove proxy " + proxy_elt.getName() +
                     " and all its aliases?",
                  "Unit-less Proxy Removal", JOptionPane.YES_NO_OPTION,
                  JOptionPane.QUESTION_MESSAGE
               );

            if ( option == JOptionPane.YES_OPTION )
            {
               ConfigBroker broker = new ConfigBrokerProxy();
               ConfigContext ctx   = proxy_info.getContext();
               List resources      = ctx.getResources();

               for ( Iterator a = proxy_info.getAliases().iterator();
                     a.hasNext(); )
               {
                  ConfigElement alias_elt = (ConfigElement) a.next();
                  broker.remove(ctx, alias_elt);
               }

               broker.remove(ctx, proxy_elt);
               removed_vertices.add(proxy_cell);
            }
         }

         DeviceGraphModel model = (DeviceGraphModel) this.graph.getModel();
         model.remove(removed_edges.toArray(), removed_vertices.toArray(),
                      false);

         // Remove port as a child of our cell.
         DefaultGraphCell device_cell = (DefaultGraphCell) mView.getCell();
         device_cell.remove(port);

         UnitInfo old_unit_info = (UnitInfo) port.getUserObject();
         UnitTypeGroup ug =
            (UnitTypeGroup) mUnitGroups.get(old_unit_info.getUnitType());

         // Account for the unit removal from its group.
         ug.unitCount--;

         // Now, we need to update all the other ports that represent units
         // that are (sequentially) after the port that has been removed.
         // The ports need to have their contained Integer decremented, and
         // all proxies referring to each of those ports needs to have its
         // UNIT_PROPERTY updated to the new unit value.
         Integer unit_val = old_unit_info.getUnitNumber();

         for ( Enumeration children = device_cell.children();
               children.hasMoreElements(); )
         {
            Object child = children.nextElement();

            if ( child instanceof DefaultPort )
            {
               DefaultPort child_port = (DefaultPort) child;
               UnitInfo unit_info     = (UnitInfo) child_port.getUserObject();

               // If unit_value is less than child_port's unit number, then we
               // know that child_port's unit number comes after unit_value
               // sequentially.  Therefore, it needs to be updated to have its
               // Integer value decremented by one.
               if ( unit_val.compareTo(unit_info.getUnitNumber()) < 0 )
               {
                  unit_info.decrementUnitNumber();

                  // Update any proxies referring to the unit that has had its
                  // index decremented.
                  for ( Iterator i = this.graph.getModel().edges(child_port);
                        i.hasNext(); )
                  {
                     Edge edge = (Edge) i.next();
                     DefaultPort proxy_port = (DefaultPort) edge.getSource();
                     DefaultGraphCell proxy_cell =
                        (DefaultGraphCell) proxy_port.getParent();
                     ProxyInfo proxy_info =
                        (ProxyInfo) proxy_cell.getUserObject();
                     proxy_info.getElement().setProperty(
                        EditorConstants.UNIT_PROPERTY, 0,
                        unit_info.getUnitNumber(), proxy_info.getContext()
                     );
                  }

                  // Update the labels for the remaining units to reflect that
                  // they have been shifted up one.
                  Class property_type =
                     mDeviceInfo.getUnitPropertyType(unit_info.getUnitType());
                  if ( property_type == ConfigElement.class )
                  {
                     List comp_list = (List) mUnitRowMap.get(unit_info);

                     for ( Iterator comp = comp_list.iterator();
                           comp.hasNext(); )
                     {
                        Object comp_obj = comp.next();

                        if ( comp_obj instanceof JLabel )
                        {
                           setLabelText((JLabel) comp_obj, unit_info);
                        }
                     }
                  }
               }
            }
         }

         // Change the preferred size of the renderer to account for the row
         // removal.
         pref_size = new Dimension(width, height);
         this.setPreferredSize(pref_size);

         this.revalidate();

         // Resize the cell view now that the renderer has changed.
         GraphHelpers.autoSizeCellView(this.graph, pref_size, mView);
      }

      private void removeUnit(int unitNumber, Integer unitType)
      {
         try
         {
            // XXX: This is a hack.  We know that propertyValueRemoved() or
            // propertyValueChanged() is going to be invoked as a result of one
            // of the two code paths below.  Having this member variable gives
            // us an easy way to figure out exactly which unit was removed.
            // This is particularly necessary for the propertyValueChanged()
            // case since we are decrementing a value rather than removing a
            // whole value.  I simply couldn't come up with a more elegant,
            // more robust way to handle the propertyValueChanged case().
            // -PH 3/22/2005
            mRemovedUnitInfo = new UnitInfo(unitType, new Integer(unitNumber));

            mDeviceInfo.removeUnit(unitType, unitNumber);
         }
         catch (IllegalArgumentException ex)
         {
            mRemovedUnitInfo = null;
         }
      }

      private transient CellView   mView       = null;
      private transient DeviceInfo mDeviceInfo = null;
      private transient Map        mUnitGroups = new HashMap();

      /**
       * Maps from a UnitInfo object to a List object.  The List instance
       * contains the JComponents in the row that is identified uniquely by
       * the UnitInfo key.
       */
      private transient Map mUnitRowMap = new HashMap();

      /**
       * This variable is used for a hack to communicate information between
       * removeUnit() and propertyValueChanged() indirectly.
       */
      private transient UnitInfo mRemovedUnitInfo = null;

      /**
       * A stack of all the unit rows added to this renderer dynamically
       * through user interaction.  The stack contains objects of type
       * <code>UnitRow</code>.  This is used in conjunction with the
       * mRemovedUnitInfo hack.
       */
      private transient Stack mAddedUnitRows = new Stack();

      /**
       * Keep track of whether our device supports a variable number of
       * units.  This can also be determined using the method
       * <code>DeviceInfo.hasVariableUnitCount()</code>.
       */
      private boolean mVariableUnits = false;

      private TableLayout mMainLayout = null;

      /**
       * A helper class used as the value in <code>mUnitGroups</code> that
       * assists with calculating unit numbers when adding and removing unit
       * rows from the layout.
       */
      private static class UnitTypeGroup
      {
         UnitTypeGroup(JButton addButton, int unitCount)
         {
            this.addButton = addButton;
            this.unitCount = unitCount;
         }

         public JButton addButton = null;
         public int     unitCount = 0;
      }
   }
}
