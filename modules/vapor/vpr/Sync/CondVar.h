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

#ifndef _VPR_CondVar_h_
#define _VPR_CondVar_h_

#include <vpr/vprConfig.h>

#if defined(VPR_USE_IRIX_SPROC)
#    include <ulocks.h>
#    include <vpr/Sync/CondVarGeneric.h>

namespace vpr {
    typedef CondVarGeneric CondVar;
};
#elif defined(VPR_USE_PTHREADS)
#ifndef _POSIX_C_SOURCE
#   define _POSIX_C_SOURCE VPR_POSIX_C_SOURCE
#endif

#   include <vpr/md/POSIX/Sync/CondVarPosix.h>

namespace vpr {
    typedef CondVarPosix CondVar;
};
#elif defined(VPR_USE_WIN32)
#    include <vpr/Sync/CondVarGeneric.h>
 
namespace vpr {
    typedef CondVarGeneric CondVar;
};
#else
#   include <vpr/md/NSPR/Sync/CondVarNSPR.h>

namespace vpr {
    typedef CondVarNSPR CondVar;
};
#endif   /* VPR_IRIX_SPROC */

#endif
