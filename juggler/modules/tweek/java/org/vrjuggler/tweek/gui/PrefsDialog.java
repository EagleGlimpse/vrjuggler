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

import java.awt.*;
import java.awt.event.*;
import java.util.Iterator;
import java.util.List;
import javax.swing.*;
import javax.swing.border.*;
import org.vrjuggler.tweek.services.GlobalPreferencesService;


/**
 * @since 1.0
 */
public class PrefsDialog extends JDialog
{
   public PrefsDialog (Frame owner, String title,
                       GlobalPreferencesService prefs)
   {
      super(owner, title);

      mPrefs = prefs;

      userLevel        = mPrefs.getUserLevel();
      lookAndFeel      = mPrefs.getLookAndFeel();
      beanViewer       = mPrefs.getBeanViewer();
      chooserStartDir  = mPrefs.getChooserStartDir();
      chooserOpenStyle = mPrefs.getChooserOpenStyle();

      try
      {
         jbInit();
      }
      catch(Exception e)
      {
         e.printStackTrace();
      }

      this.configComboBoxes();

      switch ( chooserOpenStyle )
      {
         case GlobalPreferencesService.EMACS_CHOOSER:
            mEmacsStyleButton.setSelected(true);
            break;
         case GlobalPreferencesService.WINDOWS_CHOOSER:
         default:
            mWindowsStyleButton.setSelected(true);
            break;
      }

      this.setModal(true);
      this.setLocationRelativeTo(owner);
   }

   public void display ()
   {
      this.pack();
      this.setVisible(true);
   }

   public void setBeanViewer (String v)
   {
      mPrefs.setBeanViewer(v);
   }

   public int getStatus ()
   {
      return status;
   }

   public static final int OK_OPTION     = JOptionPane.OK_OPTION;
   public static final int CANCEL_OPTION = JOptionPane.CANCEL_OPTION;
   public static final int CLOSED_OPTION = JOptionPane.CLOSED_OPTION;

   protected void processWindowEvent (WindowEvent e)
   {
      super.processWindowEvent(e);

      if (e.getID() == WindowEvent.WINDOW_CLOSING)
      {
         status = CLOSED_OPTION;
      }
   }

   private void jbInit() throws Exception
   {
      mFileChooserBorder = new TitledBorder(BorderFactory.createEtchedBorder(Color.white,new Color(142, 142, 142)),"File Chooser Configuration");
      mGenBorder = new TitledBorder(BorderFactory.createEtchedBorder(Color.white,new Color(142, 142, 142)),"General Configuration");
      mViewerBox.setMinimumSize(new Dimension(126, 10));
      mViewerBox.setPreferredSize(new Dimension(130, 10));

      mLevelBox.setMinimumSize(new Dimension(126, 10));
      mLevelBox.setPreferredSize(new Dimension(130, 10));

      mOkButton.setMnemonic('O');
      mOkButton.setText("OK");
      mOkButton.addActionListener(new ActionListener()
      {
         public void actionPerformed (ActionEvent e)
         {
            okButtonAction(e);
         }
      });

      mSaveButton.setMnemonic('S');
      mSaveButton.setText("Save");
      mSaveButton.addActionListener(new ActionListener()
      {
         public void actionPerformed (ActionEvent e)
         {
            saveButtonAction(e);
         }
      });

      mCancelButton.setMnemonic('C');
      mCancelButton.setText("Cancel");
      mCancelButton.addActionListener(new ActionListener()
      {
         public void actionPerformed (ActionEvent e)
         {
            cancelButtonAction(e);
         }
      });

      mContentLayout.setColumns(1);
      mContentLayout.setRows(0);
      mContentPanel.setLayout(mContentLayout);
      mFileChooserPanel.setLayout(mFileChooserLayout);
      mGenConfigPanel.setLayout(mGenConfigLayout);
      mGeneralPanel.setBorder(mGenBorder);
      mGeneralPanel.setToolTipText("General Tweek interface configuration");
      mGeneralPanel.setLayout(mGenLayout);
      mFileChooserPanel.setBorder(mFileChooserBorder);
      mFcConfigPanel.setLayout(mFcConfigLayout);
      mFcStartDirLabel.setMaximumSize(new Dimension(24, 13));
      mFcStartDirLabel.setMinimumSize(new Dimension(24, 13));
      mFcStartDirLabel.setPreferredSize(new Dimension(24, 13));
      mFcStartDirLabel.setHorizontalAlignment(SwingConstants.RIGHT);
      mFcStartDirLabel.setText("Start Directory");
      mFcStartDirBox.setMinimumSize(new Dimension(126, 10));
      mFcStartDirBox.setPreferredSize(new Dimension(130, 10));
      mFcStartDirBox.setEditable(true);
      mFcOpenStyleTitleLabel.setMaximumSize(new Dimension(24, 13));
      mFcOpenStyleTitleLabel.setMinimumSize(new Dimension(24, 13));
      mFcOpenStyleTitleLabel.setPreferredSize(new Dimension(24, 13));
      mFcOpenStyleTitleLabel.setHorizontalAlignment(SwingConstants.RIGHT);
      mFcOpenStyleTitleLabel.setText("Open Style");
      mEmacsStyleButton.setMinimumSize(new Dimension(109, 15));
      mEmacsStyleButton.setPreferredSize(new Dimension(109, 15));
      mEmacsStyleButton.setText("Emacs Style");
      mEmacsStyleButton.addActionListener(new ActionListener()
         {
            public void actionPerformed (ActionEvent e)
            {
               chooserOpenStyle = GlobalPreferencesService.EMACS_CHOOSER;
            }
         });

      mWindowsStyleButton.setMinimumSize(new Dimension(128, 15));
      mWindowsStyleButton.setPreferredSize(new Dimension(128, 15));
      mWindowsStyleButton.setText("Windows Style");
      mWindowsStyleButton.addActionListener(new ActionListener()
         {
            public void actionPerformed (ActionEvent e)
            {
               chooserOpenStyle = GlobalPreferencesService.WINDOWS_CHOOSER;
            }
         });

      mFcOpenStyleButtonPanel.setLayout(mFcOpenStyleButtonLayout);
      mFcOpenStyleButtonLayout.setColumns(1);
      mFcOpenStyleButtonLayout.setRows(0);
      mLafLabel.setMaximumSize(new Dimension(74, 13));
      mLafLabel.setMinimumSize(new Dimension(24, 13));
      mLafLabel.setPreferredSize(new Dimension(24, 13));
      mLafLabel.setHorizontalAlignment(SwingConstants.RIGHT);
      mLevelLabel.setMinimumSize(new Dimension(24, 13));
      mLevelLabel.setPreferredSize(new Dimension(24, 13));
      mLevelLabel.setHorizontalAlignment(SwingConstants.RIGHT);
      mViewerLabel.setMaximumSize(new Dimension(24, 13));
      mViewerLabel.setMinimumSize(new Dimension(24, 13));
      mViewerLabel.setPreferredSize(new Dimension(24, 13));
      mViewerLabel.setHorizontalAlignment(SwingConstants.RIGHT);
      mLafBox.setMinimumSize(new Dimension(126, 10));
      mLafBox.setPreferredSize(new Dimension(130, 10));
      mFcOpenStyleButtonPanel.setMinimumSize(new Dimension(128, 50));
      mFcOpenStyleButtonPanel.setPreferredSize(new Dimension(128, 50));
      mLazyInstanceButton.setSelected(mPrefs.getLazyPanelBeanInstantiation());
      mLazyInstanceButton.setText("Lazy Panel Bean Instantiaion");
      mGeneralPanel.add(mGenConfigPanel, BorderLayout.CENTER);
      mContentPanel.add(mGeneralPanel, null);
      mContentPanel.add(mFileChooserPanel, null);

      mLevelLabel.setText("User Level");

      mGenConfigPanel.add(mLevelLabel,     new GridBagConstraints(0, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 3), 95, 23));
      mGenConfigPanel.add(mLevelBox,   new GridBagConstraints(1, 1, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 64, 14));

      mLafLabel.setText("Look and Feel");
      mGenConfigPanel.add(mLafLabel,    new GridBagConstraints(0, 2, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 3), 95, 23));
      mGenConfigPanel.add(mLafBox,   new GridBagConstraints(1, 2, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 64, 14));

      mViewerLabel.setText("Bean Viewer");
      mGenConfigPanel.add(mViewerLabel,     new GridBagConstraints(0, 3, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 3), 95, 23));
      mGenConfigPanel.add(mViewerBox,   new GridBagConstraints(1, 3, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 64, 14));
      mGenConfigPanel.add(mLazyInstanceButton,   new GridBagConstraints(1, 0, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));

      mButtonPanel.add(mOkButton, null);
      mButtonPanel.add(mSaveButton, null);
      mButtonPanel.add(mCancelButton, null);

      this.getContentPane().add(mContentPanel, BorderLayout.CENTER);
      this.getContentPane().add(mButtonPanel, BorderLayout.SOUTH);
      mFileChooserPanel.add(mFcConfigPanel, BorderLayout.CENTER);
      mFcConfigPanel.add(mFcStartDirLabel,       new GridBagConstraints(0, 0, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 95, 23));
      mFcConfigPanel.add(mFcStartDirBox,        new GridBagConstraints(1, 0, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 64, 14));
      mFcConfigPanel.add(mFcOpenStyleTitleLabel,             new GridBagConstraints(0, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 3), 95, 23));
      mFcConfigPanel.add(mFcOpenStyleButtonPanel,           new GridBagConstraints(1, 1, 1, 1, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 66, 0));
      mFcOpenStyleButtonPanel.add(mWindowsStyleButton, null);
      mFcOpenStyleButtonPanel.add(mEmacsStyleButton, null);
      mOpenStyleButtonGroup.add(mWindowsStyleButton);
      mOpenStyleButtonGroup.add(mEmacsStyleButton);
   }

   /**
    * Adds items to all the combo boxes.  In some cases, other modifications
    * to a combo box will be made.  These can include setting the default
    * selected value and/or adding action listeners.
    */
   private void configComboBoxes ()
   {
      // ----------------------------------------------------------------------
      // Add all the known Bean viewers to mViewerBox.
      java.util.Vector viewers = mPrefs.getBeanViewers();

      for ( int i = 0; i < viewers.size(); i++ )
      {
         mViewerBox.addItem(viewers.elementAt(i));
      }

      mViewerBox.setSelectedItem(mPrefs.getBeanViewer());
      mViewerBox.addActionListener(new ActionListener()
         {
            public void actionPerformed (ActionEvent e)
            {
               beanViewer = (String) mViewerBox.getSelectedItem();
            }
         });
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      // Add user level options to mLevelBox.
      for ( int i = GlobalPreferencesService.MIN_USER_LEVEL;
            i <= GlobalPreferencesService.MAX_USER_LEVEL;
            i++ )
      {
         mLevelBox.addItem(String.valueOf(i));
      }

      mLevelBox.setSelectedIndex(mPrefs.getUserLevel() - 1);
      mLevelBox.addActionListener(new ActionListener()
         {
            public void actionPerformed (ActionEvent e)
            {
               userLevel = mLevelBox.getSelectedIndex() + 1;
            }
         });
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      // Handle the Look-and-Feel box.
      mLafBox.addItem("Metal");
      mLafBox.addItem("Motif");
      mLafBox.addItem("Windows");
      mLafBox.addItem("Mac");
      mLafBox.addItem("System");
      mLafBox.addActionListener(new ActionListener()
         {
            public void actionPerformed (ActionEvent e)
            {
               String val = (String) mLafBox.getSelectedItem();

               if ( val.equals("Metal") )
               {
                  lookAndFeel = "javax.swing.plaf.metal.MetalLookAndFeel";
               }
               else if ( val.equals("Motif") )
               {
                  lookAndFeel = "com.sun.java.swing.plaf.motif.MotifLookAndFeel";
               }
               else if ( val.equals("Windows") )
               {
                  lookAndFeel = "com.sun.java.swing.plaf.windows.WindowsLookAndFeel";
               }
               else if ( val.equals("Macintosh") )
               {
                  lookAndFeel = "com.sun.java.swing.plaf.mac.MacLookAndFeel";
               }
               else
               {
                  lookAndFeel = UIManager.getSystemLookAndFeelClassName();
               }
            }
         });
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      // Handle the file chooser starting directory box stuff.
      Iterator i = GlobalPreferencesService.getStartDirList().iterator();

      boolean has_start_dir = false;

      while ( i.hasNext() )
      {
         String dir = (String) i.next();

         if ( dir.equals(chooserStartDir) )
         {
            has_start_dir = true;
         }

         mFcStartDirBox.addItem(dir);
      }

      if ( ! has_start_dir )
      {
         mFcStartDirBox.addItem(chooserStartDir);
      }

      mFcStartDirBox.setSelectedItem(chooserStartDir);

      mFcStartDirBox.addActionListener(new ActionListener()
         {
            public void actionPerformed (ActionEvent e)
            {
               chooserStartDir = (String) mFcStartDirBox.getSelectedItem();
            }
         });
      // ----------------------------------------------------------------------
   }

   private void okButtonAction (ActionEvent e)
   {
      status = OK_OPTION;
      commit();
      mPrefs.save(false);
      setVisible(false);
   }

   private void saveButtonAction (ActionEvent e)
   {
      commit();
      mPrefs.save(false);
   }

   private void cancelButtonAction (ActionEvent e)
   {
      status = CANCEL_OPTION;
      setVisible(false);
   }

   private void commit ()
   {
      mPrefs.setUserLevel(userLevel);
      mPrefs.setLookAndFeel(lookAndFeel);
      mPrefs.setBeanViewer(beanViewer);
      mPrefs.setChooserStartDir(chooserStartDir);
      mPrefs.setChooserOpenStyle(chooserOpenStyle);
      mPrefs.setLazyPanelBeanInstantiation(mLazyInstanceButton.isSelected());
   }

   private int status;

   private int    userLevel        = 0;
   private String lookAndFeel      = null;
   private String beanViewer       = null;
   private String chooserStartDir  = GlobalPreferencesService.DEFAULT_START;
   private int    chooserOpenStyle = GlobalPreferencesService.DEFAULT_CHOOSER;

   private GlobalPreferencesService mPrefs = null;

   private JPanel     mContentPanel  = new JPanel();
   private GridLayout mContentLayout = new GridLayout();

   private JPanel       mGeneralPanel     = new JPanel();
   private TitledBorder mGenBorder;
   private BorderLayout mGenLayout        = new BorderLayout();

   private JPanel        mGenConfigPanel  = new JPanel();
   private GridBagLayout mGenConfigLayout = new GridBagLayout();

   private JLabel    mLevelLabel  = new JLabel();
   private JComboBox mLevelBox    = new JComboBox();
   private JLabel    mLafLabel    = new JLabel();
   private JComboBox mLafBox      = new JComboBox();
   private JLabel    mViewerLabel = new JLabel();
   private JComboBox mViewerBox   = new JComboBox();

   private JPanel       mFileChooserPanel = new JPanel();
   private TitledBorder mFileChooserBorder;
   private BorderLayout mFileChooserLayout = new BorderLayout();

   private JPanel        mFcConfigPanel  = new JPanel();
   private GridBagLayout mFcConfigLayout = new GridBagLayout();

   private JLabel       mFcStartDirLabel         = new JLabel();
   private JComboBox    mFcStartDirBox           = new JComboBox();
   private JLabel       mFcOpenStyleTitleLabel   = new JLabel();
   private JPanel       mFcOpenStyleButtonPanel  = new JPanel();
   private GridLayout   mFcOpenStyleButtonLayout = new GridLayout();
   private ButtonGroup  mOpenStyleButtonGroup    = new ButtonGroup();
   private JRadioButton mWindowsStyleButton      = new JRadioButton();
   private JRadioButton mEmacsStyleButton        = new JRadioButton();

   private JPanel  mButtonPanel  = new JPanel();
   private JButton mCancelButton = new JButton();
   private JButton mSaveButton   = new JButton();
   private JButton mOkButton     = new JButton();
   private JCheckBox mLazyInstanceButton = new JCheckBox();
}
