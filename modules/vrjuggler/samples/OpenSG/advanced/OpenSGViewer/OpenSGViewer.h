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

#ifndef _OSG_VIEWER_
#define _OSG_VIEWER_

/* ---- Feature list ----------

- Viewplatform
- Pointing grabbing
- Grabbing using pinch gloves
- Slicing using cubic mouse interfacing

--------------------------
*/                    

#include <iostream>
#include <iomanip>

/*-----------------------------Juggler includes-------------------------------*/
#include <vrj/vrjConfig.h>
#include <vrj/Draw/OGL/GlApp.h>
#include <vrj/Draw/OpenSG/OpenSGApp.h>

#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/Generate.h>

#include <gadget/Type/PositionInterface.h>
#include <gadget/Type/AnalogInterface.h>
#include <gadget/Type/DigitalInterface.h>

#include <vrj/Draw/OGL/GlContextData.h>

// Common viewer helper classes
#include <User.h>

/*-----------------------------OpenSG includes--------------------------------*/
#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGTransform.h>
/*----------------------------------------------------------------------------*/


class OpenSGViewer : public vrj::OpenSGApp
{
public:
    OpenSGViewer(vrj::Kernel* kern)
       : vrj::OpenSGApp(kern), velocity(0.0f)
    {
        std::cout << "OpenSGViewer::OpenSGViewer called\n";
        mFileToLoad = std::string("");
   }

    virtual ~OpenSGViewer (void)
    {
        std::cout << "OpenSGViewer::~OpenSGViewer called\n";
    }

    // Handle any initialization needed before API
    virtual void init();
        
    /** Initialize the scene graph */
    virtual void initScene();

    /** Return the scene root for this application */
    virtual OSG::NodePtr getSceneRoot()
    { return mSceneRoot; }

    virtual void draw();

    virtual void contextInit();

    virtual void preFrame();

    void setModelFileName(std::string filename)
    {
        std::cout << "OpenSGViewer::setModelFileName: Set filename: [" << filename << "]\n";
        mFileToLoad = filename;
    }

  private:
    void initGLState();

  private:
    std::string         mFileToLoad;      /**< Filename of the file to load */

    User                mUser;            /**< The user that this is a viewer for */

    //   mSceneRoot:[mSceneTransform]
    //         |
    //   mLightNode:[DirectionalLight]
    //         |
    //   mLightBeacon:[Transform]
    //         |
    //     mModelRoot
    OSG::NodePtr        mSceneRoot;       /**< The root of the scene */
    OSG::TransformPtr   mSceneTransform;  /**< Transform core */
    OSG::NodePtr        mModelRoot;       /**< Root of the loaded model */

    OSG::NodePtr        mLightNode;       /**< Light node to use */
    OSG::NodePtr        mLightBeacon;     /**< A beacon for the light */

  public:
    gadget::PositionInterface  mWandPos;     /**< The position of the wand */
    gadget::DigitalInterface   mButton0;     /**< Wand button 0 */
    gadget::DigitalInterface   mButton1;     /**< Wand button 1 */
    gadget::DigitalInterface   mButton2;     /**< Wand button 2 */
    float  velocity;
};


#endif
