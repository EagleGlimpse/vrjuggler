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

#include <tweek/tweekConfig.h>

#include <vpr/Sync/Guard.h>
#include <vpr/Util/Debug.h>

#include <tweek/Util/Debug.h>
#include <tweek/CORBA/CorbaManager.h>
#include <tweek/CORBA/SubjectImpl.h>
#include <tweek/CORBA/SubjectManagerImpl.h>


namespace tweek
{

void SubjectManagerImpl::registerSubject (SubjectImpl* subject_servant,
                                          const char* name)
{
   std::string name_str(name);

   m_subject_ids_mutex.acquire();
   {
      // We have to register servant with POA first.
      m_subject_ids[name_str] =
         m_corba_mgr.getChildPOA()->activate_object(subject_servant);
   }
   m_subject_ids_mutex.release();

   registerSubject(subject_servant->_this(), name_str);
}

vpr::ReturnStatus SubjectManagerImpl::unregisterSubject (const char* name)
{
   vpr::ReturnStatus status;
   std::string name_str(name);
   vpr::Guard<vpr::Mutex> guard(m_subjects_mutex);

   if ( m_subjects.count(name_str) > 0 )
   {
      m_subject_ids_mutex.acquire();
      {
         // Deactivate the object in the POA.
         m_corba_mgr.getChildPOA()->deactivate_object(m_subject_ids[name_str]);
      }
      m_subject_ids_mutex.release();

      Subject_ptr subj = m_subjects[name_str];
      CORBA::release(subj);
      m_subjects.erase(name_str);
   }
   else
   {
      vprDEBUG(vprDBG_ALL, vprDBG_CRITICAL_LVL)
         << "ERROR: No subject registered under the name '" << name_str
         << "'\n" << vprDEBUG_FLUSH;
      status.setCode(vpr::ReturnStatus::Fail);
   }

   return status;
}

void SubjectManagerImpl::registerSubject (Subject_ptr subject,
                                          const std::string& name)
{
   vpr::Guard<vpr::Mutex> guard(m_subjects_mutex);

   vprDEBUG(tweekDBG_CORBA, vprDBG_STATE_LVL)
      << "Registering subject named '" << name << "'\n" << vprDEBUG_FLUSH;

   m_subjects[name] = Subject::_duplicate(subject);
}

Subject_ptr SubjectManagerImpl::getSubject (const char* name)
{
   Subject_ptr subject;
   std::string name_str(name);
   vpr::Guard<vpr::Mutex> guard(m_subjects_mutex);

   subject_map_t::iterator i;

   i = m_subjects.find(name_str);

   if ( i != m_subjects.end() )
   {
      subject = Subject::_duplicate((*i).second);
      vprDEBUG(tweekDBG_CORBA, vprDBG_STATE_LVL)
         << "Returning subject named '" << name << "'\n" << vprDEBUG_FLUSH;
   }
   else
   {
      subject = Subject::_nil();
      vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
         << "No registered subject named '" << name << "'!\n"
         << vprDEBUG_FLUSH;
   }

   return subject;
}

tweek::SubjectManager::SubjectList* SubjectManagerImpl::getAllSubjects ()
{
   subject_map_t::iterator i;
   CORBA::ULong j;

   vprDEBUG(tweekDBG_CORBA, vprDBG_STATE_LVL)
      << "Constructing sequence of subjects to return to caller ...\n"
      << vprDEBUG_FLUSH;

   // Create the sequence and size it.
   tweek::SubjectManager::SubjectList* subjects =
      new tweek::SubjectManager::SubjectList();
   subjects->length(m_subjects.size());

   vprDEBUG(tweekDBG_CORBA, vprDBG_VERB_LVL)
      << "Sequence size: " << subjects->length() << std::endl
      << vprDEBUG_FLUSH;

   for ( i = m_subjects.begin(), j = 0; i != m_subjects.end(); i++, j++ )
   {
      tweek::SubjectManager::RegisteredSubject rs;
      rs.subject_name = CORBA::string_dup((*i).first.c_str());
      rs.subject_ref  = Subject::_duplicate((*i).second);

      vprDEBUG(tweekDBG_CORBA, vprDBG_VERB_LVL)
         << "Adding subject[" << j << "]: " << rs.subject_name << std::endl
         << vprDEBUG_FLUSH;

      (*subjects)[j] = rs;
   }

   vprDEBUG(tweekDBG_CORBA, vprDBG_STATE_LVL)
      << "Returning all subjects to caller\n" << vprDEBUG_FLUSH;

   return subjects;
}

} // End of tweek namespace
