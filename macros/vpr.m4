dnl ************* <auto-copyright.pl BEGIN do not edit this line> *************
dnl
dnl VR Juggler is (C) Copyright 1998, 1999, 2000 by Iowa State University
dnl
dnl Original Authors:
dnl   Allen Bierbaum, Christopher Just,
dnl   Patrick Hartling, Kevin Meinert,
dnl   Carolina Cruz-Neira, Albert Baker
dnl
dnl This library is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU Library General Public
dnl License as published by the Free Software Foundation; either
dnl version 2 of the License, or (at your option) any later version.
dnl
dnl This library is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl Library General Public License for more details.
dnl
dnl You should have received a copy of the GNU Library General Public
dnl License along with this library; if not, write to the
dnl Free Software Foundation, Inc., 59 Temple Place - Suite 330,
dnl Boston, MA 02111-1307, USA.
dnl
dnl -----------------------------------------------------------------
dnl File:          $RCSfile$
dnl Date modified: $Date$
dnl Version:       $Revision$
dnl -----------------------------------------------------------------
dnl
dnl ************** <auto-copyright.pl END do not edit this line> **************

dnl ---------------------------------------------------------------------------
dnl VPR_PATH([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl
dnl Test for VPR and then define VPR_CFLAGS, VPR_CXXFLAGS and VPR_LIBS.
dnl ---------------------------------------------------------------------------
AC_DEFUN(VPR_PATH,
[
    dnl Get the cflags and libraries from the vpr-config script
    AC_ARG_WITH(vpr-prefix,
                [  --with-vpr-prefix=PFX   Prefix where VPR is installed (optional)],
                vpr_config_prefix="$withval", vpr_config_prefix="")
    AC_ARG_WITH(vpr-exec-prefix,
                [  --with-vpr-exec-prefix=PFX Exec prefix where VPR is installed (optional)],
                vpr_config_exec_prefix="$withval", vpr_config_exec_prefix="")
    AC_ARG_ENABLE(vprtest, [  --disable-vprtest       Do not try to compile and run a test VPR program],
		    , enable_vprtest=yes)

dnl    for module in . $4
dnl    do
dnl        case "$module" in
dnl           <fill me in!>) 
dnl               vpr_config_args="$vpr_config_args ???"
dnl           ;;
dnl        esac
dnl    done

    if test x$vpr_config_exec_prefix != x ; then
        vpr_config_args="$vpr_config_args --exec-prefix=$vpr_config_exec_prefix"

        if test x${VPR_CONFIG+set} != xset ; then
            VPR_CONFIG=$vpr_config_exec_prefix/bin/vpr-config
        fi
    fi

    if test x$vpr_config_prefix != x ; then
        vpr_config_args="$vpr_config_args --prefix=$vpr_config_prefix"

        if test x${VPR_CONFIG+set} != xset ; then
            VPR_CONFIG=$vpr_config_prefix/bin/vpr-config
        fi
    fi

    AC_PATH_PROG(VPR_CONFIG, vpr-config, no)
    min_vpr_version=ifelse([$1], ,0.0.1,$1)
dnl    AC_MSG_CHECKING(for VPR - version >= $min_vpr_version)
    no_vpr=''
    if test "x$VPR_CONFIG" = "xno" ; then
        no_vpr=yes
    else
        VPR_CFLAGS=`$VPR_CONFIG $vpr_config_args --cflags`
        VPR_CXXFLAGS=`$VPR_CONFIG $vpr_config_args --cxxflags`
        VPR_LIBS=`$VPR_CONFIG $vpr_config_args --libs`
dnl        vpr_config_major_version=`$VPR_CONFIG $vpr_config_args --version | \
dnl               sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
dnl        vpr_config_minor_version=`$VPR_CONFIG $vpr_config_args --version | \
dnl               sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
dnl        vpr_config_micro_version=`$VPR_CONFIG $vpr_config_args --version | \
dnl               sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    fi

    if test "x$no_vpr" != x ; then
        if test "$VPR_CONFIG" = "no" ; then
            echo "*** The vpr-config script installed by VPR could not be found"
            echo "*** If VPR was installed in PREFIX, make sure PREFIX/bin is in"
            echo "*** your path, or set the VPR_CONFIG environment variable to the"
            echo "*** full path to vpr-config."
        fi
        VPR_CFLAGS=""
        VPR_CXXFLAGS=""
        VPR_LIBS=""
        ifelse([$3], , :, [$3])
    fi
    AC_SUBST(VPR_CFLAGS)
    AC_SUBST(VPR_CXXFLAGS)
    AC_SUBST(VPR_LIBS)
])
