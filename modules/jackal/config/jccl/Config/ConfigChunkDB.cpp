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

#include <jccl/jcclConfig.h>

#include <sys/types.h>

#include <jccl/Config/ConfigChunkDB.h>
#include <jccl/Config/ConfigChunk.h>
#include <jccl/Config/ChunkFactory.h>
#include <jccl/Config/ParseUtil.h>
#include <jccl/Config/ConfigTokens.h>
#include <jccl/Config/ConfigIO.h>
#include <jccl/Util/Debug.h>


namespace jccl
{


ConfigChunkDB::ConfigChunkDB (): chunks()
{
   ;
}

ConfigChunkDB::~ConfigChunkDB ()
{
   // Any ConfigChunks where the only copy is owned by this db
   // are automagically deleted here.
}

ConfigChunkDB::ConfigChunkDB (ConfigChunkDB& db): chunks()
{
   *this = db;
}

ConfigChunkDB& ConfigChunkDB::operator = (const ConfigChunkDB& db)
{
   unsigned int i, size;
   chunks.clear();
   for ( i = 0, size = db.chunks.size(); i < size; i++ )
   {
      chunks.push_back (db.chunks[i]);
      //chunks.push_back (new ConfigChunk(*(db.chunks[i])));
   }
   return *this;
}

ConfigChunkPtr ConfigChunkDB::getChunk (const std::string& name) const
{
   /* returns a chunk with the given name, if such exists, or NULL.
    */
   unsigned int i, size;
   for ( i = 0, size = chunks.size(); i < size; i++ )
   {
      if ( !vjstrcasecmp (name, chunks[i]->getProperty("name")) )
      {
         return chunks[i];
      }
   }
   return ConfigChunkPtr();
}

std::vector<ConfigChunkPtr> ConfigChunkDB::getChunks() const
{
   return chunks;
}

void ConfigChunkDB::addChunks(std::vector<ConfigChunkPtr> new_chunks)
{
   // no! must make copies of all chunks. sigh...
   unsigned int i, size;
   for ( i = 0, size = new_chunks.size(); i < size; i++ )
   {
      addChunk (new_chunks[i]);
   }

   //addChunk (new ConfigChunk (*new_chunks[i]));
}

void ConfigChunkDB::addChunks(const ConfigChunkDB *db)
{
   addChunks (db->chunks);
}

void ConfigChunkDB::addChunk(ConfigChunkPtr new_chunk)
{
   removeNamed (new_chunk->getProperty("Name"));
   chunks.push_back (new_chunk);
}

std::vector<ConfigChunkPtr>* ConfigChunkDB::getOfDescToken (const std::string& mytypename) const
{
   std::vector<ConfigChunkPtr>* v = new std::vector<ConfigChunkPtr>;

   for ( unsigned int i = 0; i < chunks.size(); i++ )
   {
      if ( !vjstrcasecmp (mytypename, chunks[i]->getProperty("type")) )
      {
         v->push_back(chunks[i]);
      }
   }
   return v;
}

void ConfigChunkDB::clear ()
{
   chunks.clear();
}

//: Sorts the chunks based on dependancies of chunk ptrs
//! PRE: we need a "good object"
//! MODIFIES: self.  We move the objects around so they are sorted
//! ARGS: auxChunks - Auxilary chunks that have been loaded already
//! POST: Topologically sorted
// Copy the chunks over to a new list.  Repetatively try to
// find an item in the source list that already has it's dependencies
// copied into the dst list.  Do this iteratively until done or
// until fail.
int ConfigChunkDB::dependencySort(ConfigChunkDB* auxChunks)
{
   // Print out dependancies
#ifdef JCCL_DEBUG
   vprDEBUG_BEGIN(jcclDBG_CONFIG,4) << "---- Dependencies -----------\n" << vprDEBUG_FLUSH;

   for ( unsigned int i=0;i<chunks.size();i++ )
   {
      vprDEBUG(jcclDBG_CONFIG,4) << "Chunk:" << chunks[i]->getProperty("name")
                                 << std::endl << "\tDepends on:\n"
                                 << vprDEBUG_FLUSH;
      std::vector<std::string> deps = chunks[i]->getChunkPtrDependencies();

      if ( deps.size() > 0 )
      {
         for ( unsigned int j=0;j<deps.size();j++ )
         {
            vprDEBUG(jcclDBG_CONFIG,4) << "   " << j << ": "
                                       << deps[j].c_str() << std::endl
                                       << vprDEBUG_FLUSH;
         }
      }
      else
      {
         vprDEBUG(jcclDBG_CONFIG,4) << "   Nothing.\n" << vprDEBUG_FLUSH;
      }
   }
   vprDEBUG_END(jcclDBG_CONFIG,4) << "-----------------------------\n" << vprDEBUG_FLUSH;
#endif

   // --- THE SORT --- //
   // Create new src list to work from
   // Targetting the local data
   // So basically, we take an element from the src list one at a time
   // If it's dependencies are already in the local list, add it to the local list
   // else go on to the next one
   // Kinda like an insertion sort
   std::vector<ConfigChunkPtr> src_chunks = chunks;
   chunks = std::vector<ConfigChunkPtr>(0);        // Chunks is the local data - Zero it out to start

   bool dep_pass(true);             // Flag for Pass dependency check
   std::vector<std::string> deps;   // Dependencies of current item
   std::vector<ConfigChunkPtr>::iterator cur_item = src_chunks.begin();          // The current src item to look at

   while ( cur_item != src_chunks.end() )
   {          // While not at end of src list
      vprDEBUG(jcclDBG_CONFIG,4)
         << "Checking depencies for: " << (*cur_item)->getProperty("name")
         << "\n" << vprDEBUG_FLUSH;

      dep_pass = true;
      deps = (*cur_item)->getChunkPtrDependencies();             // Get src dependencies
      for ( unsigned int dep_num=0; (dep_num < deps.size()) && dep_pass; dep_num++ )
      {  // For each dependency

         if ( ConfigChunk::hasSeparator (deps[dep_num]) )
         {
            std::string chunkname = ConfigChunk::getFirstNameComponent(deps[dep_num]);
            bool found = false;
            ConfigChunkPtr ch = getChunk(chunkname);
            found = ((ch.get() != 0) && (ch->getEmbeddedChunk (deps[dep_num]).get() != 0));
            if ( !found && auxChunks )
            {
               ch = auxChunks->getChunk(chunkname);
               found = ((ch.get() != 0) && (ch->getEmbeddedChunk (deps[dep_num]).get() != 0));
            }
            dep_pass = dep_pass && found;
         }
         else
         {
            dep_pass = dep_pass && (getChunk(deps[dep_num]).get() ||
                                    (auxChunks && auxChunks->getChunk(deps[dep_num]).get()));

         }
      }

      if ( dep_pass )
      {        // If all dependencies are accounted for
         addChunk(*cur_item);        // Copy src to dst
         src_chunks.erase(cur_item);         // Erase it from source
         cur_item = src_chunks.begin();      // Goto first item
      }
      else
      {
         cur_item++;             // Try next item
      }
   }

   // ASSERT: (Done with sort)
   //   Either, all depencies have been accounted for and the src list is empty
   //   OR we went all the way through list without finding an item that passes

   if ( src_chunks.size() > 0 )
   {     // Items left, so we failed to get all dependencies
      // ouput error
      for ( unsigned int i=0;i<src_chunks.size();i++ )
      {
         vprDEBUG(vprDBG_ERROR,0)
            << clrOutNORM(clrRED, "ERROR:") << " Dependency error:  Chunk:"
            << src_chunks[i]->getProperty("name") << "\tDepends on: \n"
            << vprDEBUG_FLUSH;

         std::vector<std::string> deps = src_chunks[i]->getChunkPtrDependencies();
         if ( deps.size() > 0 )
         {
            for ( unsigned int j=0;j<deps.size();j++ )
            {
               vprDEBUG(vprDBG_ERROR,0) << "\tdep " << j << ": "
                                        << deps[j].c_str() << std::endl
                                        << vprDEBUG_FLUSH;
            }
         }
         else
         {
            vprDEBUG(vprDBG_ERROR,0) << "Nothing.\n" << vprDEBUG_FLUSH;
         }

         vprDEBUG(vprDBG_ERROR, vprDBG_CRITICIAL_LVL)
            << clrOutNORM(clrRED, "Check for missing files or missing chunks in loaded files.\n")
            << vprDEBUG_FLUSH;
      }
      chunks.insert(chunks.end(), src_chunks.begin(), src_chunks.end());   // Copy over the rest anyway

      return -1;
   }
   else
   {
      // Print out sorted dependancies
#ifdef JCCL_DEBUG

      vprDEBUG_BEGIN(jcclDBG_CONFIG,4) << "---- After sort ----"
                                       << std::endl << vprDEBUG_FLUSH;

      for ( unsigned int i=0; i<chunks.size(); i++ )
      {
         vprDEBUG(jcclDBG_CONFIG,4) << "Chunk:"
                                    << chunks[i]->getProperty("name")
                                    << "\n\tDepends on:\n"
                                    << vprDEBUG_FLUSH;

         std::vector<std::string> deps = chunks[i]->getChunkPtrDependencies();
         if ( deps.size() > 0 )
         {
            for ( unsigned int j=0;j<deps.size();j++ )
            {
               vprDEBUG(jcclDBG_CONFIG,4) << "   " << j << ": "
                                          << deps[j].c_str()
                                          << std::endl << vprDEBUG_FLUSH;
            }
         }
         else
         {
            vprDEBUG(jcclDBG_CONFIG,4) << "   Nothing.\n"<< vprDEBUG_FLUSH;
         }
      }

      vprDEBUG_END(jcclDBG_CONFIG,4) << "-----------------\n"
                                     << vprDEBUG_FLUSH;
#endif

      return 0;      // Success
   }
}

/* IO functions: */

std::ostream& operator << (std::ostream& out, const ConfigChunkDB& self)
{
   ConfigIO::instance()->writeConfigChunkDB (out, self);
   return out;
}

std::istream& operator >> (std::istream& in, ConfigChunkDB& self)
{
   ConfigIO::instance()->readConfigChunkDB (in, self);
   return in;
}

bool ConfigChunkDB::load (const std::string& filename, const std::string& parentfile)
{

   file_name = demangleFileName (filename, parentfile);
   vprDEBUG(jcclDBG_CONFIG,3) << "ConfigChunkDB::load(): opening file "
                              << file_name.c_str() << " -- " << vprDEBUG_FLUSH;
   bool retval = ConfigIO::instance()->readConfigChunkDB (file_name, *this);

   vprDEBUG(jcclDBG_CONFIG,3) << " finished.. read " << chunks.size()
                              << " chunks\n" << vprDEBUG_FLUSH;
   return retval;
}

bool ConfigChunkDB::save (const std::string& file_name) const
{
   vprDEBUG(jcclDBG_CONFIG,3) << "ConfigChunkDB::save(): saving file "
                              << file_name.c_str() << " -- " << vprDEBUG_FLUSH;

   bool retval = ConfigIO::instance()->writeConfigChunkDB (file_name, *this);
   if ( retval )
   {
      vprDEBUG(jcclDBG_CONFIG,3) << " finished.\n" << vprDEBUG_FLUSH;
   }
   else
   {
      vprDEBUG(vprDBG_ERROR,1)
         << clrOutNORM(clrRED, "ERROR:") << " ConfigChunkDB::save() - "
         << "Unable to open file '"
         << file_name.c_str() << "'\n" << vprDEBUG_FLUSH;
   }
   return retval;
}

bool ConfigChunkDB::isEmpty() const
{
   return chunks.empty();
}

void ConfigChunkDB::removeAll()
{
   clear();
}

bool ConfigChunkDB::removeNamed (const std::string& name)
{
   iterator cur_chunk = chunks.begin();
   while ( cur_chunk != chunks.end() )
   {
      VarValue v = ((*cur_chunk)->getProperty("Name"));
      if ( (v.getType() == T_STRING)
           &&  (!vjstrcasecmp (name, (std::string)v)) )
      {
         cur_chunk = chunks.erase(cur_chunk);
         return true;
      }
      else
      {
         cur_chunk++;
      }
   }

   return false;
}

} // End of jccl namespace
