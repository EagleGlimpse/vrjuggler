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

package org.vrjuggler.vrjconfig.customeditors.display_window;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.text.AttributeSet;
import javax.swing.text.BadLocationException;
import javax.swing.text.PlainDocument;
import info.clearthought.layout.*;

import org.vrjuggler.tweek.services.EnvironmentService;
import org.vrjuggler.tweek.services.EnvironmentServiceProxy;
import org.vrjuggler.jccl.config.*;
import org.vrjuggler.jccl.editors.PropertyEditorPanel;


public class DisplayWindowStartDialog
   extends JDialog
   implements EditorConstants
{
   public DisplayWindowStartDialog(Container parent, ConfigContext ctx,
                                   Dimension resolution)
      throws HeadlessException
   {
      super();
      this.setTitle("Basic Display Window Parameters");
      this.setModal(true);
      enableEvents(AWTEvent.WINDOW_EVENT_MASK);

      mResolution = resolution;
      mVisualIdField.setDocument(new HexidecimalDocument());

      ConfigBroker cfg_broker = new ConfigBrokerProxy();
      ConfigDefinition my_def =
         cfg_broker.getRepository().get(INPUT_AREA_TYPE);

      // Create a dummy config element to hand off to new PropertyEditorPanel
      // instances.
      ConfigElementFactory factory =
         new ConfigElementFactory(cfg_broker.getRepository().getAllLatest());
      ConfigElement win_elt = factory.createUnique(my_def, ctx);

      Object value = win_elt.getProperty(KEYBOARD_MOUSE_PTR_PROPERTY, 0);
      PropertyDefinition prop_def =
         my_def.getPropertyDefinition(KEYBOARD_MOUSE_PTR_PROPERTY);

      mKbdDevEditor = new PropertyEditorPanel(ctx, value, prop_def, win_elt,
                                              0, Color.white);

      value    = win_elt.getProperty(LOCK_KEY_PROPERTY, 0);
      prop_def = my_def.getPropertyDefinition(LOCK_KEY_PROPERTY);

      mLockKeyEditor = new PropertyEditorPanel(ctx, value, prop_def, win_elt,
                                               0, Color.white);

      // Hang onto our dummy config element so that we can get back the value
      // of the lock key later.
      mWinElement = win_elt;

      try
      {
         jbInit();
         setupButtonPanel();
         addResources(ctx);

         setSpinnerModel(mRedDepthSpinner, 1, 1, 8);
         setSpinnerModel(mGreenDepthSpinner, 1, 1, 8);
         setSpinnerModel(mBlueDepthSpinner, 1, 1, 8);
         setSpinnerModel(mAlphaDepthSpinner, 1, 1, 8);
         setSpinnerModel(mDepthBufferSpinner, 1, 1, 32);

         mSleepTimeField.setValue(
            mWinElement.getProperty(SLEEP_TIME_PROPERTY, 0)
         );

         Object start_locked =
            mWinElement.getProperty(START_LOCKED_PROPERTY, 0);
         mStartLockedCB.setSelected(Boolean.TRUE.equals(start_locked));

         // Validate the default values for the various text fields.
         validateUserInput();
      }
      catch(Exception e)
      {
         e.printStackTrace();
      }

      this.getRootPane().setDefaultButton(mOkButton);

      this.setResizable(false);
      this.pack();
      this.setLocationRelativeTo(parent);
   }

   public DisplayWindowStartDialog(Container parent, ConfigContext ctx,
                                   ConfigElement winElt, Dimension resolution)
      throws HeadlessException
   {
      super();
      this.setTitle(winElt.getName() + " Properties");
      this.setModal(true);
      enableEvents(AWTEvent.WINDOW_EVENT_MASK);

      mWinElement = winElt;
      mResolution = resolution;
      mVisualIdField.setDocument(new HexidecimalDocument());

      PropertyDefinition prop_def =
         winElt.getDefinition().getPropertyDefinition(KEYBOARD_MOUSE_PTR_PROPERTY);
      mKbdDevEditor = 
         new PropertyEditorPanel(ctx,
                                 winElt.getProperty(KEYBOARD_MOUSE_PTR_PROPERTY, 0),
                                 prop_def, winElt, 0, Color.white);

      prop_def =
         winElt.getDefinition().getPropertyDefinition(LOCK_KEY_PROPERTY);
      mLockKeyEditor =
         new PropertyEditorPanel(ctx, winElt.getProperty(LOCK_KEY_PROPERTY, 0),
                                 prop_def, winElt, 0, Color.white);

      try
      {
         jbInit();
         setupButtonPanel();
         addResources(ctx);

         mResourceLabel.setEnabled(false);
         mResourceChooser.setEnabled(false);

         mNameField.setText(winElt.getName());

         ConfigElement fb_cfg =
            (ConfigElement) winElt.getProperty("frame_buffer_config", 0);

         int val;
         val = ((Integer) fb_cfg.getProperty("red_size", 0)).intValue();
         setSpinnerModel(mRedDepthSpinner, val, 1, 8);
         val = ((Integer) fb_cfg.getProperty("green_size", 0)).intValue();
         setSpinnerModel(mGreenDepthSpinner, val, 1, 8);
         val = ((Integer) fb_cfg.getProperty("blue_size", 0)).intValue();
         setSpinnerModel(mBlueDepthSpinner, val, 1, 8);
         val = ((Integer) fb_cfg.getProperty("alpha_size", 0)).intValue();
         setSpinnerModel(mAlphaDepthSpinner, val, 1, 8);
         val = ((Integer) fb_cfg.getProperty("depth_buffer_size", 0)).intValue();
         setSpinnerModel(mDepthBufferSpinner, val, 1, 32);

         mVisualIdField.setText(((Integer) fb_cfg.getProperty("visual_id", 0)).toString());
         mFSAACheckbox.setSelected(fb_cfg.getProperty("fsaa_enable", 0).equals(Boolean.TRUE));

         mPositionXField.setValue(winElt.getProperty(ORIGIN_PROPERTY, 0));
         mPositionYField.setValue(winElt.getProperty(ORIGIN_PROPERTY, 1));
         mWidthField.setValue(winElt.getProperty(SIZE_PROPERTY, 0));
         mHeightField.setValue(winElt.getProperty(SIZE_PROPERTY, 1));

         mStereoCheckbox.setSelected(winElt.getProperty("stereo", 0).equals(Boolean.TRUE));
         mBorderCheckbox.setSelected(winElt.getProperty("border", 0).equals(Boolean.TRUE));
         mHideMouseCheckbox.setSelected(winElt.getProperty("hide_mouse", 0).equals(Boolean.TRUE));

         mSleepTimeField.setValue(winElt.getProperty(SLEEP_TIME_PROPERTY, 0));
         Object start_locked = winElt.getProperty(START_LOCKED_PROPERTY, 0);
         mStartLockedCB.setSelected(Boolean.TRUE.equals(start_locked));

         // Validate the default values for the various text fields.
         validateUserInput();
      }
      catch(Exception e)
      {
         e.printStackTrace();
      }

      this.getRootPane().setDefaultButton(mOkButton);

      this.setResizable(false);
      this.pack();
      this.setLocationRelativeTo(parent);
   }

   private void setSpinnerModel(JSpinner spinner, int value, int min, int max)
   {
      SpinnerNumberModel model =
         new SpinnerNumberModel(new Integer(value), new Integer(min),
                                new Integer(max), new Integer(1));
      spinner.setModel(model);
   }

   public int showDialog()
   {
      setVisible(true);
      return status;
   }

   public DataSource getResource()
   {
      String resource_name = (String) mResourceChooser.getSelectedItem();
      return (new ConfigBrokerProxy()).get(resource_name);
   }

   public String getDisplayWindowTitle()
   {
      return mNameField.getText();
   }

   public Rectangle getDisplayWindowBounds()
   {
      int x = ((Number) mPositionXField.getValue()).intValue();
      int y = ((Number) mPositionYField.getValue()).intValue();
      int width = ((Number) mWidthField.getValue()).intValue();
      int height = ((Number) mHeightField.getValue()).intValue();
      return new Rectangle(x, y, width, height);
   }

   public Integer getVisualID()
   {
      return Integer.valueOf(mVisualIdField.getText());
   }

   public Integer getRedDepth()
   {
      return (Integer) mRedDepthSpinner.getModel().getValue();
   }

   public Integer getGreenDepth()
   {
      return (Integer) mGreenDepthSpinner.getModel().getValue();
   }

   public Integer getBlueDepth()
   {
      return (Integer) mBlueDepthSpinner.getModel().getValue();
   }

   public Integer getAlphaDepth()
   {
      return (Integer) mAlphaDepthSpinner.getModel().getValue();
   }

   public Integer getDepthBufferSize()
   {
      return (Integer) mDepthBufferSpinner.getModel().getValue();
   }

   public Boolean getFSAAEnable()
   {
      return (mFSAACheckbox.isSelected() ? Boolean.TRUE : Boolean.FALSE);
   }

   public Boolean hasBorder()
   {
      return (mBorderCheckbox.isSelected() ? Boolean.TRUE : Boolean.FALSE);
   }

   public Boolean inStereo()
   {
      return (mStereoCheckbox.isSelected() ? Boolean.TRUE : Boolean.FALSE);
   }

   public Boolean hideMouse()
   {
      return (mHideMouseCheckbox.isSelected() ? Boolean.TRUE : Boolean.FALSE);
   }

   public Object getKeyboardMousePointer()
   {
      return mWinElement.getProperty(KEYBOARD_MOUSE_PTR_PROPERTY, 0);
   }

   public Object getLockKey()
   {
      return mWinElement.getProperty(LOCK_KEY_PROPERTY, 0);
   }

   public Boolean shouldStartLocked()
   {
      return (mStartLockedCB.isSelected() ? Boolean.TRUE : Boolean.FALSE);
   }

   public Integer getSleepTime()
   {
      return (Integer) mSleepTimeField.getValue();
   }

   public static final int OK_OPTION     = JOptionPane.OK_OPTION;
   public static final int CANCEL_OPTION = JOptionPane.CANCEL_OPTION;
   public static final int CLOSED_OPTION = JOptionPane.CLOSED_OPTION;

   protected void processWindowEvent(WindowEvent e)
   {
      if (e.getID() == WindowEvent.WINDOW_CLOSING)
      {
         status = CLOSED_OPTION;
         dispose();
      }

      super.processWindowEvent(e);
   }

   private void jbInit() throws Exception
   {
      // Columns are as follows:
      //    0 -> panel start
      //    1 -> window name label
      //    2 -> spacer between window name label and window name field
      //    3 -> window name field start, lame space for mResourceChooser
      //    4 -> window name field end
      //    5 -> panel end
      double[][] main_size =
         {
            {5, TableLayout.MINIMUM, 5, 300, TableLayout.FILL, 5},
            {TableLayout.PREFERRED, TableLayout.PREFERRED,
             TableLayout.PREFERRED, TableLayout.PREFERRED,
             TableLayout.PREFERRED}
         };
      mMainLayout = new TableLayout(main_size);

      double[][] bounds_size =
         {
            {TableLayout.PREFERRED, TableLayout.PREFERRED},
            {TableLayout.PREFERRED, TableLayout.PREFERRED}
         };
      mBoundsPanelLayout = new TableLayout(bounds_size);

      double[][] fb_size =
         {
            {TableLayout.MINIMUM, 5, TableLayout.PREFERRED},
            {TableLayout.PREFERRED}
         };
      mFrameBufferPanelLayout = new TableLayout(fb_size);

      double[][] fb_settings_size =
         {
            {TableLayout.FILL, TableLayout.PREFERRED},
            {TableLayout.PREFERRED, TableLayout.PREFERRED,
             TableLayout.PREFERRED, TableLayout.PREFERRED,
             TableLayout.PREFERRED, TableLayout.PREFERRED,
             TableLayout.PREFERRED}
         };
      mFBSettingsLayout = new TableLayout(fb_settings_size);

      // Colums are as follows:
      //    0 -> editor label
      //    1 -> spacer betweeen label and editor
      //    2 -> start for lock key and sleep time editors; end for checkbox
      //         editor
      //    3 -> end for sleep time editor
      //    4 -> spacer between sleep time editor and sleep time units label
      //    5 -> end for lock key editor; unit label for sleep time editor
      //
      // NOTE: Column 2 is a hack to make it appear as though the check box
      // is two separate UI components.
      double[][] window_props_size =
         {
            {TableLayout.PREFERRED, 5, 20, TableLayout.MINIMUM, 5,
             TableLayout.MINIMUM},
            {TableLayout.PREFERRED, TableLayout.PREFERRED,
             TableLayout.PREFERRED, TableLayout.PREFERRED,
             TableLayout.PREFERRED, TableLayout.PREFERRED,
             TableLayout.PREFERRED}
         };
      mWindowPropsPanelLayout = new TableLayout(window_props_size);

      mBoundsPanelBorder =
         new TitledBorder(BorderFactory.createEtchedBorder(Color.white,
                                                           new Color(161, 161, 161)),
                                                           "Window Bounds");
      mFrameBufferPanelBorder =
         new TitledBorder(BorderFactory.createEtchedBorder(Color.white,
                                                           new Color(142, 142, 142)),
                                                           "Visual Quality Settings");
      mWindowPropsPanelBorder =
         new TitledBorder(BorderFactory.createEtchedBorder(Color.white,
                                                           new Color(142, 142, 142)),
                                                           "Window Behavior Settings");
      mFBSettingsPanelBorder =
         new EtchedBorder(EtchedBorder.RAISED, Color.white,
                          new Color(142, 142, 142));
      mMainPanel.setLayout(mMainLayout);
      mResourceLabel.setHorizontalAlignment(SwingConstants.TRAILING);
      mResourceLabel.setLabelFor(mResourceChooser);
      mResourceLabel.setText("Resource");
      mNameLabel.setHorizontalAlignment(SwingConstants.TRAILING);
      mNameLabel.setLabelFor(mNameField);
      mNameLabel.setText("Window Name");
      mNameField.setMinimumSize(new Dimension(60, 21));
      mNameField.setPreferredSize(new Dimension(60, 21));
      mNameField.setText("Display Window");
      mSizeLabel.setHorizontalAlignment(SwingConstants.TRAILING);
      mSizeLabel.setLabelFor(mSizePanel);
      mSizeLabel.setText("Size");
      mPositionLabel.setHorizontalAlignment(SwingConstants.TRAILING);
      mPositionLabel.setLabelFor(mPositionPanel);
      mPositionLabel.setText("Position");
      mWidthField.setMinimumSize(new Dimension(50, 21));
      mWidthField.setPreferredSize(new Dimension(50, 21));
      mWidthField.setValue(new Integer(200));
      mWidthField.setHorizontalAlignment(SwingConstants.TRAILING);
      mSizeXLabel.setText("\u00D7");
      mHeightField.setMinimumSize(new Dimension(50, 21));
      mHeightField.setPreferredSize(new Dimension(50, 21));
      mHeightField.setValue(new Integer(200));
      mPositionXField.setMinimumSize(new Dimension(40, 21));
      mPositionXField.setPreferredSize(new Dimension(40, 21));
      mPositionXField.setValue(new Integer(0));
      mPositionXField.setHorizontalAlignment(SwingConstants.TRAILING);
      mPositionXLabel.setText(",");
      mPositionYField.setMinimumSize(new Dimension(40, 21));
      mPositionYField.setPreferredSize(new Dimension(40, 21));
      mPositionYField.setValue(new Integer(0));
      mPositionYField.setHorizontalAlignment(SwingConstants.TRAILING);
      mFBSettingsLabel.setHorizontalAlignment(SwingConstants.TRAILING);
      mFBSettingsLabel.setLabelFor(mFBSettingsPanel);
      mFBSettingsLabel.setText("OpenGL Frame Buffer Settings");
      mFBSettingsPanel.setLayout(mFBSettingsLayout);
      mRedDepthLabel.setHorizontalAlignment(SwingConstants.TRAILING);
      mRedDepthLabel.setLabelFor(mRedDepthSpinner);
      mRedDepthLabel.setText("Red Depth");
      mGreenDepthLabel.setHorizontalAlignment(SwingConstants.TRAILING);
      mGreenDepthLabel.setLabelFor(mGreenDepthSpinner);
      mGreenDepthLabel.setText("Green Depth");
      mBlueDepthLabel.setHorizontalAlignment(SwingConstants.TRAILING);
      mBlueDepthLabel.setLabelFor(mBlueDepthSpinner);
      mBlueDepthLabel.setText("Blue Depth");
      mAlphaDepthLabel.setHorizontalAlignment(SwingConstants.TRAILING);
      mAlphaDepthLabel.setLabelFor(mAlphaDepthSpinner);
      mAlphaDepthLabel.setText("Alpha Depth");
      mDepthBufferLabel.setHorizontalAlignment(SwingConstants.TRAILING);
      mDepthBufferLabel.setLabelFor(mDepthBufferSpinner);
      mDepthBufferLabel.setText("Depth Buffer Size");
      mBorderCheckbox.setHorizontalTextPosition(SwingConstants.LEFT);
      mBorderCheckbox.setSelected(true);
      mBorderCheckbox.setText("Has border");
      mStereoCheckbox.setHorizontalTextPosition(SwingConstants.LEFT);
      mStereoCheckbox.setText("Render in stereo");
      mHideMouseCheckbox.setHorizontalTextPosition(SwingConstants.LEFT);
      mHideMouseCheckbox.setText("Hide mouse inside window");
      mKbdDevLabel.setHorizontalAlignment(SwingConstants.TRAILING);
      mKbdDevLabel.setText("Keyboard/Mouse");
      mLockKeyLabel.setHorizontalAlignment(SwingConstants.TRAILING);
      mLockKeyLabel.setText("Lock Key");
      mStartLockedCB.setText("Start Locked");
      mStartLockedCB.setHorizontalTextPosition(SwingConstants.LEFT);
      mSleepTimeLabel.setHorizontalAlignment(SwingConstants.TRAILING);
      mSleepTimeLabel.setLabelFor(mSleepTimeField);
      mSleepTimeLabel.setText("Sleep Time");
      mSleepTimeField.setHorizontalAlignment(SwingConstants.RIGHT);
      mSleepMSLabel.setText("ms");
      mOkButton.setEnabled(false);
      mOkButton.setMnemonic('0');
      mOkButton.setSelected(false);
      mOkButton.setText("OK");
      mOkButton.addActionListener(new DisplayWindowStartDialog_mOkButton_actionAdapter(this));
      mCancelButton.setMnemonic('0');
      mCancelButton.setText("Cancel");
      mCancelButton.addActionListener(new DisplayWindowStartDialog_mCancelButton_actionAdapter(this));
      mHelpButton.setEnabled(false);
      mHelpButton.setMnemonic('0');
      mHelpButton.setText("Help");
      mHelpButton.addActionListener(new DisplayWindowStartDialog_mHelpButton_actionAdapter(this));
      mFSAACheckbox.setText("Enable full-screen anti-aliasing");
      mBoundsPanel.setLayout(mBoundsPanelLayout);
      mBoundsPanel.setBorder(mBoundsPanelBorder);
      mFrameBufferPanel.setLayout(mFrameBufferPanelLayout);
      mFrameBufferPanel.setBorder(mFrameBufferPanelBorder);
      mWindowPropsPanel.setLayout(mWindowPropsPanelLayout);
      mWindowPropsPanel.setBorder(mWindowPropsPanelBorder);
      mFBSettingsPanel.setBorder(mFBSettingsPanelBorder);
      mButtonPanel.setLayout(mButtonPanelLayout);
      mVisualIdLabel.setHorizontalAlignment(SwingConstants.TRAILING);
      mVisualIdLabel.setLabelFor(mVisualIdField);
      mVisualIdLabel.setText("Visual ID");
      mVisualIdField.setMinimumSize(new Dimension(19, 20));
      mVisualIdField.setPreferredSize(new Dimension(28, 20));
      mVisualIdField.setToolTipText("Enter a GLX or WGL visual ID in hexadecimal notation or -1 to ignore");
      mVisualIdField.setText("-1");
      mVisualIdField.addActionListener(
         new DisplayWindowStartDialog_mVisualIdField_actionAdapter(this)
      );
      mVisualIdField.addFocusListener(
         new DisplayWindowStartDialog_mVisualIdField_focusAdapter(this)
      );
      this.getContentPane().add(mMainPanel, BorderLayout.CENTER);
      mBoundsPanel.add(mSizeLabel, new TableLayoutConstraints(0, 0, 0, 0,
                                                            TableLayout.RIGHT,
                                                            TableLayout.CENTER));
      mBoundsPanel.add(mSizePanel, new TableLayoutConstraints(1, 0, 1, 0,
                                                            TableLayout.LEFT,
                                                            TableLayout.CENTER));
      mBoundsPanel.add(mPositionLabel,
                       new TableLayoutConstraints(0, 1, 0, 1,
                                                  TableLayout.RIGHT,
                                                  TableLayout.CENTER));
      mBoundsPanel.add(mPositionPanel,
                       new TableLayoutConstraints(1, 1, 1, 1,
                                                  TableLayout.LEFT,
                                                  TableLayout.CENTER));
      mPositionPanel.add(mPositionXField, null);
      mPositionPanel.add(mPositionXLabel, null);
      mPositionPanel.add(mPositionYField, null);
      mSizePanel.add(mWidthField, null);
      mSizePanel.add(mSizeXLabel, null);
      mSizePanel.add(mHeightField, null);
      mFrameBufferPanel.setPreferredSize(new Dimension(450, 200));
      mFrameBufferPanel.add(mFBSettingsLabel,
                            new TableLayoutConstraints(0, 0, 0, 0,
                                                       TableLayout.LEFT,
                                                       TableLayout.CENTER));
      mFrameBufferPanel.add(mFBSettingsPanel,
                            new TableLayoutConstraints(2, 0, 2, 0,
                                                       TableLayout.FULL,
                                                       TableLayout.FULL));
      mFBSettingsPanel.add(mVisualIdLabel,
                           new TableLayoutConstraints(0, 0, 0, 0,
                                                      TableLayout.FULL,
                                                      TableLayout.CENTER));
      mFBSettingsPanel.add(mVisualIdField,
                           new TableLayoutConstraints(1, 0, 1, 0,
                                                      TableLayout.FULL,
                                                      TableLayout.CENTER));
      mFBSettingsPanel.add(mRedDepthLabel,
                           new TableLayoutConstraints(0, 1, 0, 1,
                                                      TableLayout.FULL,
                                                      TableLayout.CENTER));
      mFBSettingsPanel.add(mRedDepthSpinner,
                           new TableLayoutConstraints(1, 1, 1, 1,
                                                      TableLayout.FULL,
                                                      TableLayout.CENTER));
      mFBSettingsPanel.add(mGreenDepthLabel,
                           new TableLayoutConstraints(0, 2, 0, 2,
                                                      TableLayout.FULL,
                                                      TableLayout.CENTER));
      mFBSettingsPanel.add(mGreenDepthSpinner,
                           new TableLayoutConstraints(1, 2, 1, 2,
                                                      TableLayout.FULL,
                                                      TableLayout.CENTER));
      mFBSettingsPanel.add(mBlueDepthLabel,
                           new TableLayoutConstraints(0, 3, 0, 3,
                                                      TableLayout.FULL,
                                                      TableLayout.CENTER));
      mFBSettingsPanel.add(mBlueDepthSpinner,
                           new TableLayoutConstraints(1, 3, 1, 3,
                                                      TableLayout.FULL,
                                                      TableLayout.CENTER));
      mFBSettingsPanel.add(mAlphaDepthLabel,
                           new TableLayoutConstraints(0, 4, 0, 4,
                                                      TableLayout.FULL,
                                                      TableLayout.CENTER));
      mFBSettingsPanel.add(mAlphaDepthSpinner,
                           new TableLayoutConstraints(1, 4, 1, 4,
                                                      TableLayout.FULL,
                                                      TableLayout.CENTER));
      mFBSettingsPanel.add(mDepthBufferLabel,
                           new TableLayoutConstraints(0, 5, 0, 5,
                                                      TableLayout.FULL,
                                                      TableLayout.CENTER));
      mFBSettingsPanel.add(mDepthBufferSpinner,
                           new TableLayoutConstraints(1, 5, 1, 5,
                                                      TableLayout.FULL,
                                                      TableLayout.CENTER));
      mFBSettingsPanel.add(mFSAACheckbox,
                           new TableLayoutConstraints(0, 6, 1, 6,
                                                      TableLayout.LEFT,
                                                      TableLayout.CENTER));
      mWindowPropsPanel.add(mBorderCheckbox,
                     new TableLayoutConstraints(0, 0, 2, 0,
                                                TableLayout.RIGHT,
                                                TableLayout.CENTER));
      mWindowPropsPanel.add(mStereoCheckbox,
                     new TableLayoutConstraints(0, 1, 2, 1,
                                                TableLayout.RIGHT,
                                                TableLayout.CENTER));
      mWindowPropsPanel.add(mHideMouseCheckbox,
                     new TableLayoutConstraints(0, 2, 2, 2,
                                                TableLayout.RIGHT,
                                                TableLayout.CENTER));
      mWindowPropsPanel.add(mKbdDevLabel,
                            new TableLayoutConstraints(0, 3, 0, 3,
                                                       TableLayoutConstraints.RIGHT,
                                                       TableLayoutConstraints.CENTER));
      mWindowPropsPanel.add(mKbdDevEditor,
                            new TableLayoutConstraints(2, 3, 5, 3,
                                                       TableLayoutConstraints.LEFT,
                                                       TableLayoutConstraints.CENTER));
      mWindowPropsPanel.add(mLockKeyLabel,
                            new TableLayoutConstraints(0, 4, 0, 4,
                                                       TableLayoutConstraints.RIGHT,
                                                       TableLayoutConstraints.CENTER));
      mWindowPropsPanel.add(mLockKeyEditor,
                            new TableLayoutConstraints(2, 4, 5, 4,
                                                       TableLayoutConstraints.LEFT,
                                                       TableLayoutConstraints.CENTER));
      mWindowPropsPanel.add(mStartLockedCB,
                            new TableLayoutConstraints(0, 5, 2, 5,
                                                       TableLayoutConstraints.RIGHT,
                                                       TableLayoutConstraints.CENTER));
      mWindowPropsPanel.add(mSleepTimeLabel,
                            new TableLayoutConstraints(0, 6, 0, 6,
                                                       TableLayoutConstraints.RIGHT,
                                                       TableLayoutConstraints.CENTER));
      mWindowPropsPanel.add(mSleepTimeField,
                            new TableLayoutConstraints(2, 6, 3, 6,
                                                       TableLayoutConstraints.FULL,
                                                       TableLayoutConstraints.CENTER));
      mWindowPropsPanel.add(mSleepMSLabel,
                            new TableLayoutConstraints(5, 6, 5, 6,
                                                       TableLayoutConstraints.LEFT,
                                                       TableLayoutConstraints.CENTER));
      mMainPanel.add(mResourceLabel,
                     new TableLayoutConstraints(1, 0, 1, 0,
                                                TableLayout.RIGHT,
                                                TableLayout.CENTER));
      mMainPanel.add(mResourceChooser,
                     new TableLayoutConstraints(3, 0, 3, 0,
                                                TableLayout.LEFT,
                                                TableLayout.CENTER));
      mMainPanel.add(mNameLabel,
                     new TableLayoutConstraints(1, 1, 1, 1,
                                                TableLayout.RIGHT,
                                                TableLayout.CENTER));
      mMainPanel.add(mNameField,
                     new TableLayoutConstraints(3, 1, 4, 1,
                                                TableLayout.FULL,
                                                TableLayout.CENTER));
      mMainPanel.add(mBoundsPanel,
                     new TableLayoutConstraints(0, 2, 5, 2,
                                                TableLayout.FULL,
                                                TableLayout.FULL));
      mMainPanel.add(mFrameBufferPanel,
                     new TableLayoutConstraints(0, 3, 5, 3,
                                                TableLayout.FULL,
                                                TableLayout.FULL));
      mMainPanel.add(mWindowPropsPanel,
                     new TableLayoutConstraints(0, 4, 5, 4,
                                                TableLayout.FULL,
                                                TableLayout.FULL));
      this.getContentPane().add(mButtonPanel, BorderLayout.SOUTH);
   }

   private void setupButtonPanel()
   {
      if ( (new EnvironmentServiceProxy()).getOS() == EnvironmentService.Windows )
      {
         mButtonPanelLayout.setAlignment(FlowLayout.CENTER);
         mButtonPanel.add(mOkButton, null);
         mButtonPanel.add(mCancelButton, null);
         mButtonPanel.add(mHelpButton, null);
      }
      else
      {
         mButtonPanelLayout.setAlignment(FlowLayout.RIGHT);
         mButtonPanel.add(mHelpButton, null);
         mButtonPanel.add(mCancelButton, null);
         mButtonPanel.add(mOkButton, null);
      }
   }

   private void addResources(ConfigContext ctx)
   {
      for ( java.util.Iterator i = ctx.getResources().iterator();
            i.hasNext(); )
      {
         mResourceChooser.addItem(i.next());
      }
   }

   private int status = CANCEL_OPTION;
   private ConfigElement mWinElement = null;
   private Dimension mResolution = null;

   private JPanel mMainPanel = new JPanel();
   private TableLayout mMainLayout = null;
   private JLabel mResourceLabel = new JLabel();
   private JComboBox mResourceChooser = new JComboBox();
   private JLabel mNameLabel = new JLabel();
   private JTextField mNameField = new JTextField();
   private JPanel mBoundsPanel = new JPanel();
   private TableLayout mBoundsPanelLayout = null;
   private TitledBorder mBoundsPanelBorder;
   private JLabel mSizeLabel = new JLabel();
   private JPanel mSizePanel = new JPanel();
   private JLabel mPositionLabel = new JLabel();
   private JPanel mPositionPanel = new JPanel();
   private JFormattedTextField mWidthField = new JFormattedTextField();
   private JLabel mSizeXLabel = new JLabel();
   private JFormattedTextField mHeightField = new JFormattedTextField();
   private JFormattedTextField mPositionXField = new JFormattedTextField();
   private JLabel mPositionXLabel = new JLabel();
   private JFormattedTextField mPositionYField = new JFormattedTextField();
   private JPanel mFrameBufferPanel = new JPanel();
   private TableLayout mFrameBufferPanelLayout = null;
   private TitledBorder mFrameBufferPanelBorder;
   private JLabel mFBSettingsLabel = new JLabel();
   private JPanel mFBSettingsPanel = new JPanel();
   private Border mFBSettingsPanelBorder;
   private TableLayout mFBSettingsLayout = null;
   private JLabel mVisualIdLabel = new JLabel();
   private JTextField mVisualIdField = new JTextField();
   private JLabel mRedDepthLabel = new JLabel();
   private JSpinner mRedDepthSpinner = new JSpinner();
   private JLabel mGreenDepthLabel = new JLabel();
   private JSpinner mGreenDepthSpinner = new JSpinner();
   private JLabel mBlueDepthLabel = new JLabel();
   private JSpinner mBlueDepthSpinner = new JSpinner();
   private JLabel mAlphaDepthLabel = new JLabel();
   private JSpinner mAlphaDepthSpinner = new JSpinner();
   private JLabel mDepthBufferLabel = new JLabel();
   private JSpinner mDepthBufferSpinner = new JSpinner();
   private JCheckBox mFSAACheckbox = new JCheckBox();
   private JPanel mWindowPropsPanel = new JPanel();
   private TableLayout mWindowPropsPanelLayout = null;
   private TitledBorder mWindowPropsPanelBorder;
   private JCheckBox mBorderCheckbox = new JCheckBox();
   private JCheckBox mStereoCheckbox = new JCheckBox();
   private JCheckBox mHideMouseCheckbox = new JCheckBox();
   private JLabel mKbdDevLabel = new JLabel();
   private PropertyEditorPanel mKbdDevEditor = null;
   private JLabel mLockKeyLabel = new JLabel();
   private PropertyEditorPanel mLockKeyEditor = null;
   private JCheckBox mStartLockedCB = new JCheckBox();
   private JLabel mSleepTimeLabel = new JLabel();
   private JFormattedTextField mSleepTimeField = new JFormattedTextField();
   private JLabel mSleepMSLabel = new JLabel();
   private JPanel mButtonPanel = new JPanel();
   private FlowLayout mButtonPanelLayout = new FlowLayout();
   private JButton mOkButton = new JButton();
   private JButton mCancelButton = new JButton();
   private JButton mHelpButton = new JButton();

   void okPressed(ActionEvent e)
   {
      status = OK_OPTION;
      dispose();
   }

   void cancelPressed(ActionEvent e)
   {
      status = CANCEL_OPTION;
      dispose();
   }

   void helpPressed(ActionEvent e)
   {

   }

   private void validateUserInput()
   {
      boolean name_valid, width_valid, height_valid, x_pos_valid, y_pos_valid;

      // The name cannot be an empty string.
      name_valid   = ! mNameField.getText().equals("");
      width_valid  = ((Integer) mWidthField.getValue()).intValue() > 0;
      height_valid = ((Integer) mHeightField.getValue()).intValue() > 0;
      x_pos_valid  = ((Integer) mPositionXField.getValue()).intValue() >= 0;
      y_pos_valid  = ((Integer) mPositionYField.getValue()).intValue() >= 0;

      if ( name_valid && width_valid && height_valid && x_pos_valid &&
           y_pos_valid )
      {
         mOkButton.setEnabled(true);
         validateWindowBounds();
      }
      else
      {
         mOkButton.setEnabled(false);
      }
   }

   /**
    * Ensures that the bounds of the display window are within the managed
    * area of the desktop.  If this is not the case, user confirmation is
    * requested.
    *
    * @param e DocumentEvent
    */
   private void validateWindowBounds(/*DocumentEvent e*/)
   {
      int window_width  = Integer.parseInt(mWidthField.getText());
      int window_height = Integer.parseInt(mHeightField.getText());
      int window_pos_x  = Integer.parseInt(mPositionXField.getText());
      int window_pos_y  = Integer.parseInt(mPositionYField.getText());

      if ( window_pos_x < 0 || window_pos_y < 0 ||
           window_width + window_pos_x > mResolution.width ||
           window_height + window_pos_y > mResolution.height )
      {
         int answer =
            JOptionPane.showConfirmDialog(this,
                                          "Do you want the window to open outside the managed area?",
                                          "Display Window Outside Managed Area",
                                          JOptionPane.YES_NO_OPTION,
                                          JOptionPane.WARNING_MESSAGE);

         if ( answer == JOptionPane.YES_OPTION )
         {
         }
         else
         {
         }
      }
   }

   void mVisualIdField_focusLost(FocusEvent e)
   {
      validateVisualID();
   }

   void mVisualIdField_actionPerformed(ActionEvent e)
   {
      validateVisualID();
   }

   private void validateVisualID()
   {
      String id = mVisualIdField.getText();

      if ( ! id.equals("-1") && ! id.startsWith("0x") )
      {
         mVisualIdField.setText("0x" + id);
      }

      // If the value in the visual ID field is "-1", then the individual
      // frame buffer settings editors must be active.  Otherwise, they will
      // be deactivated since the values will be ignored at run time.
      boolean enabled = mVisualIdField.getText().equals("-1");
      mRedDepthLabel.setEnabled(enabled);
      mRedDepthSpinner.setEnabled(enabled);
      mGreenDepthLabel.setEnabled(enabled);
      mGreenDepthSpinner.setEnabled(enabled);
      mBlueDepthLabel.setEnabled(enabled);
      mBlueDepthSpinner.setEnabled(enabled);
      mAlphaDepthLabel.setEnabled(enabled);
      mAlphaDepthSpinner.setEnabled(enabled);
      mDepthBufferLabel.setEnabled(enabled);
      mDepthBufferSpinner.setEnabled(enabled);
      mFSAACheckbox.setEnabled(enabled);
   }

   private static class HexidecimalDocument extends PlainDocument
   {
      public void insertString(int offset, String string,
                               AttributeSet attributes)
      {
         if ( string == null )
         {
            return;
         }
         else
         {
            String new_value = "";
            int length = getLength();

            if ( length == 0 )
            {
               new_value = string;
            }
            else
            {
               try
               {
                  String currentContent = getText(0, length);
                  StringBuffer currentBuffer = new StringBuffer(currentContent);
                  currentBuffer.insert(offset, string);
                  new_value = currentBuffer.toString();
               }
               catch (BadLocationException locEx)
               {
                  locEx.printStackTrace();
               }
            }

            try
            {
               // If the user is not in the process of entering "-1" or "0x",
               // validate the input as a base 16 integer.
               if ( ! (new_value.equals("-") || new_value.equals("-1") ||
                       new_value.equals("0x")) )
               {
                  if ( new_value.startsWith("0x") )
                  {
                     new_value = new_value.substring(2);
                  }

                  Integer.parseInt(new_value, 16);
               }

               try
               {
                  super.insertString(offset, string, attributes);
               }
               catch (BadLocationException locEx)
               {
                  locEx.printStackTrace();
               }
            }
            catch (NumberFormatException exception)
            {
               Toolkit.getDefaultToolkit().beep();
            }
         }
      }
   }
}

class DisplayWindowStartDialog_mOkButton_actionAdapter
   implements ActionListener
{
   private DisplayWindowStartDialog adaptee;

   DisplayWindowStartDialog_mOkButton_actionAdapter(DisplayWindowStartDialog adaptee)
   {
      this.adaptee = adaptee;
   }
   public void actionPerformed(ActionEvent e)
   {
      adaptee.okPressed(e);
   }
}

class DisplayWindowStartDialog_mCancelButton_actionAdapter
   implements ActionListener
{
   private DisplayWindowStartDialog adaptee;

   DisplayWindowStartDialog_mCancelButton_actionAdapter(DisplayWindowStartDialog adaptee)
   {
      this.adaptee = adaptee;
   }
   public void actionPerformed(ActionEvent e)
   {
      adaptee.cancelPressed(e);
   }
}

class DisplayWindowStartDialog_mHelpButton_actionAdapter
   implements ActionListener
{
   private DisplayWindowStartDialog adaptee;

   DisplayWindowStartDialog_mHelpButton_actionAdapter(DisplayWindowStartDialog adaptee)
   {
      this.adaptee = adaptee;
   }
   public void actionPerformed(ActionEvent e)
   {
      adaptee.helpPressed(e);
   }
}

class DisplayWindowStartDialog_mVisualIdField_focusAdapter extends FocusAdapter
{
   private DisplayWindowStartDialog adaptee;

   DisplayWindowStartDialog_mVisualIdField_focusAdapter(DisplayWindowStartDialog adaptee)
   {
      this.adaptee = adaptee;
   }
   public void focusLost(FocusEvent e)
   {
      adaptee.mVisualIdField_focusLost(e);
   }
}

class DisplayWindowStartDialog_mVisualIdField_actionAdapter
   implements ActionListener
{
   private DisplayWindowStartDialog adaptee;

   DisplayWindowStartDialog_mVisualIdField_actionAdapter(DisplayWindowStartDialog adaptee)
   {
      this.adaptee = adaptee;
   }
   public void actionPerformed(ActionEvent e)
   {
      adaptee.mVisualIdField_actionPerformed(e);
   }
}
