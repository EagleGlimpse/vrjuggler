/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2003 by Iowa State University
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

#ifndef _JCCL_CONFIG_MANGER_H_
#define _JCCL_CONFIG_MANGER_H_

#include <jccl/jcclConfig.h>
#include <vector>
#include <list>

#include <vpr/Sync/Mutex.h>
#include <vpr/Sync/Guard.h>
#include <vpr/Util/Assert.h>
#include <vpr/Util/Singleton.h>
#include <vpr/DynLoad/LibraryLoader.h>

#include <jccl/Config/Configuration.h>
#include <jccl/Config/ConfigDefinitionRepository.h>
#include <jccl/Config/ConfigElement.h>

#include <jccl/Util/Debug.h>


namespace jccl
{

class ConfigElementHandler;
class RemoteReconfig;

/** Dynamic reconfiguration management plugin for JCCL.
 *  The ConfigManager provides a complete solution for configuring an
 *  application via JCCL's ConfigElement API.  The ConfigManager can
 *  configure based on static configuration files, or dynamically
 *  via a network interface to the Java front-end.
 *
 *  The ConfigManager can be used in a number of ways; it provides a
 *  complete default solution to configuration, but also exposes
 *  enough of its inner workings to allow applications to implement
 *  their own dynamic configuration algorithms.
 *
 *  The simplest way to use the ConfigManager is to create one or more
 *  objects that implement the ConfigElementHandler interface, and register
 *  these using the ConfigManager::addConfigElementHandler method.
 *
 *  Requests to add ConfigElements can be added via the JavaBeans or by the
 *  addPending*() methods of this class.  These requests are added to
 *  the ConfigManager's "pending" list.  The ConfigManager also maintains
 *  an "active" list, containing all the ConfigElements that have been
 *  successfully configured.
 *
 *  Once ConfigElementHandlers have been registered with the ConfigManager,
 *  the application should periodically call
 *  ConfigManager::attemptReconfiguration.  This will try to match items
 *  in the pending list with ConfigElementHandler objects that know how to
 *  configure them.
 *
 *  For more advanced uses, ConfigManager provides accessor functions that
 *  allow direct manipulation of the pending and active lists.  This
 *  allows an application to decide on its own when and how to process
 *  requests in the pending list.  However, the attemptReconfiguration()
 *  interface should be sufficient for almost all uses.
 *
 *  @date January 13, 2000
 */
class JCCL_CLASS_API ConfigManager
{
public:
   struct PendingElement
   {
      PendingElement() : mType(0)
      {;}

      enum { ADD=0, REMOVE=1};
      unsigned mType;           // What type of element is it (ADD or REMOVE)
      ConfigElementPtr mElement;
   };


public: // -- Query functions --- //

   /** Checks if the named ConfigElement is in the active configuration.
    *  This locks the active list to do processing.
    */
   bool isElementInActiveList(std::string elementName);

   /** Is the element of this type in the active configuration?
    *  This locks the active list to do processing.
    */
   bool isElementTypeInActiveList(std::string elementName);

   /** Is there a element of this type in the pending list??
    *  This locks the pending list to do processing
    */
   bool isElementTypeInPendingList(std::string elementType);

public:   // ----- PENDING LIST ----- //

   /** Marks pending list as "not stale".
    *  To minimize time spent on configuration calls, the Configuration
    *  Manager will mark the pending list as "stale" if it does not
    *  change for several consecutive calls of attemptReconfiguration.
    *  Call this method when something happens that might allow
    *  items on a stale pending list to be processed.
    *  <p>
    *  For example, in VR Juggler, applications (which are
    *  ConfigElementHandlers) can be explicitly changed via a vrj::Kernel
    *  method.  When this happens, the VR Juggler kernel calls
    *  refreshPendingList because the new application object may be able
    *  to process ConfigElements that the old one could not.
    *  <p>
    *  Generally, if an object is added to the system via
    *  ConfigElementHandler's addConfig method, it is not necessary to
    *  call this function explicitly; the ConfigManager will notice that
    *  the pending and active lists have changed and will consider the
    *  pending list to be fresh.
    *
    *  @see pendingNeedsChecked
    */
   void refreshPendingList();

   /** Add the ConfigElements in db to pending list as adds.
    *  The pending list must not be (already) locked.
    *  ConfigElements in db are copied.
    */
   void addPendingAdds(Configuration* db);

   /** Add the ConfigElements in db to pending list as removes.
    *  The pending list must not be (already) locked.
    *  ConfigElements in db are copied.
    */
   void addPendingRemoves(Configuration* db);

   /** Add an entry to the pending list.
    *  The pending list must not be locked.
    *  A copy of the pendingElement is placed on the pending list.
    */
   void addPending(PendingElement& pendingElement);

   /** Erases an item from the pending list.
    *  The pending list must be locked && item must be in list.
    *  Item is invalid after this operation.
    */
   void removePending(std::list<PendingElement>::iterator item);

   /** Checks if we need to check the pending list.
    *  Checks if the pending list is "fresh" or if it should be marked
    *  as "stale".  If the pending list has been checked several times
    *  without changing at all, we can assume that the elements inside of
    *  it cannot be processed by the application.
    *  This is a utility function for attemptReconfiguration.
    *  CONCURRENCY: concurrent.
    */
   bool pendingNeedsChecked();

   /** Checks to see if the Pending List is stale, meaning that the
    *  ConfigManager is not actively trying to configure anything
    *  right now.
    */
   bool isPendingStale();

   /** Locks the pending list.
    *  This function blocks until it can lock the list of pending
    *  configuration changes.
    *  The caller of this method must call unlockPending() when it
    *  is finished viewing/modifying the pending list.
    */
   void lockPending()
   {
      mPendingLock.acquire();
   }

   /** Unlocks the pending list.
    *  Unlocks the mutex held on the pending list.
    *  The caller of this method must first have called lockPending().
    */
   void unlockPending()
   {
      mPendingLock.release();
   }

   /** Get an iterator to the beginning of the pending list.
    *  The caller of this method must have locked the pending list.
    */
   std::list<PendingElement>::iterator getPendingBegin()
   {
      vprASSERT(1 == mPendingLock.test());
      return mPendingConfig.begin();
   }

   /** Get an iterator to the end of the pending list.
    *  The caller of this method must have locked the pending list.
    */
   std::list<PendingElement>::iterator getPendingEnd()
   {
      vprASSERT(1 == mPendingLock.test());
      return mPendingConfig.end();
   }

   /** Print a copy of the pending list to vprDEBUG.
    *  The caller of this method must have locked the pending list.
    */
   void debugDumpPending(int debug_level);

   /** Get the size of the pending list.
    *  CONCURRENCY: concurrent
    */
   int getNumPending()
   {
      return mPendingConfig.size();
   }

public:   // ----- ACTIVE LIST ----- //
   /** Checks if active list is empty.
    *  CONCURRENCY: concurrent
    */
   bool isActiveEmpty()
   {
      return mActiveConfig.vec().empty();
   }

   /** Locks the active list.
    *  This function blocks until it can lock the list of active
    *  ConfigElements.
    *  The caller of this method must call unlockActive() when it
    *  is finished viewing/modifying the active list.
    */
   void lockActive()
   {
      mActiveLock.acquire();
   }

   /** Unlocks the active list.
    *  The method releases the lock on the active configuration list.
    *  The caller of this method must have previously locked the active
    *  list with lockActive().
    */
   void unlockActive()
   {
      mActiveLock.release();
   }

   /** Get an iterator to the beginning of the active list.
    *  The caller of this method must have locked the active list.
    */
   std::vector<jccl::ConfigElementPtr>::iterator getActiveBegin()
   {
      vprASSERT(1 == mActiveLock.test());
      return mActiveConfig.vec().begin();
   }

   /** Get an iterator to the end of the active list.
    *  The caller of this method must have locked the active list.
    */
   std::vector<jccl::ConfigElementPtr>::iterator getActiveEnd()
   {
      vprASSERT(1 == mActiveLock.test());
      return mActiveConfig.vec().end();
   }

   /** Removes the named ConfigElement from the active list.
    *  The caller of this method must have locked the active list.
    *  If no element with a matching name is found, this method has
    *  no effect.
    */
   void removeActive(const std::string& elementName);

   /** Adds a ConfigElement to the active list.
    *  This method locks the active list; therefore, the caller
    *  MUST NOT have locked the list before calling it.
    *  This does not process the ConfigElement in anyway; it simply
    *  appends it to the active list.
    *  If a element with the same name is already in the active list,
    *  the old element is replaced by the new one.
    *  <p>
    *  This method is occasionally useful when an application wants
    *  to add items to the active list that were not created via
    *  the ConfigManager's dynamic reconfiguration ability.
    *  For example, when JCCL's network server opens a new
    *  connection, it explicitly creates a ConfigElement describing
    *  that connection and adds it to the active list with this
    *  method.
    */
   void addActive(ConfigElementPtr element);

   /** Get a pointer to the active list (as a jccl::Configuration).
    *  The caller of this method must have locked the active list.
    *  The pointer returned is only valid until the list is unlocked.
    *  CONCURRENCY: sequential
    */
   Configuration* getActiveConfig()
   {
      vprASSERT(1 == mActiveLock.test());
      return &mActiveConfig;
   }

public:
   /** Scan the active list for items that don't have their dependencies
    *  filled.
    *  Any elements in the active list with dependencies not filled are
    *  added to the pending list with a pair of entries - a remove and
    *  an equivalent add.  This way, the object will be removed from the
    *  system on the next check of the pending list, and will be re-added
    *  if its dependencies are ever subsequently met.
    *  @return The number of lost dependencies found
    */
   int scanForLostDependencies();

   /** Reset pending check count, so that pending list is made not stale.
    *  Delay config from becoming stale.  ConfigManager must try to
    *  config items in pending list again before the list can
    *  become stale.
    */
   void delayStalePendingList()
   {
     mLastPendingSize = mPendingConfig.size() + 1;
   }

   //------------ Default DynamicReconfig Handling Stuff -------------------

   void addConfigElementHandler(ConfigElementHandler* h);
   void removeConfigElementHandler(ConfigElementHandler* h);
   int attemptReconfiguration();
   //int attemptHandlerReconfiguration(ConfigElementHandler* h);

   void setRemoteReconfigPlugin(jccl::RemoteReconfig* plugin);

private:
   Configuration             mActiveConfig;  /**< Current configuration.     */
   std::list<PendingElement> mPendingConfig; /**< Pending config changes.    */
   vpr::Mutex                mPendingLock;   /**< Lock on pending list.      */
   vpr::Mutex                mActiveLock;    /**< Lock on active config list.*/

   /** List of objects that know how to handle configuration changes. */
   std::vector<ConfigElementHandler*> mElementHandlers;

   // The following variables are used to implement some logic
   // that "stales" the pending list.   (see pendingNeedsChecked)
   vpr::Mutex              mPendingCountMutex;

   /** Number of times pending list has been checked since it last changed. */
   int                     mPendingCheckCount;

   /** Size of pending list when last checked (used to check for changes). */
   int                     mLastPendingSize;

   vpr::LibraryLoader mPluginLoader;

   /** Network communications object for reconfiguration control. */
   RemoteReconfig* mReconfigIf;

   std::string              mCachedLocalHostName;  /**< A cached copy of the local host name */
protected:

   ConfigManager();
   virtual ~ConfigManager();

   /** Enables the remote runtime reconfiguration interface object. */
   void loadRemoteReconfig();

   // needed for windows:
   ConfigManager(const ConfigManager&) {;}
   void operator= (const ConfigManager&) {;}

   vprSingletonHeader(ConfigManager);

}; // class ConfigManager

} // namespace jccl


#endif
