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

#include <jccl/Util/Debug.h>
#include <vpr/Util/Assert.h>
#include <jccl/Config/ChunkDescDB.h>
#include <jccl/Config/ChunkDesc.h>
#include <jccl/Config/ParseUtil.h>
#include <jccl/Config/ConfigTokens.h>
#include <jccl/Config/ConfigIO.h>

namespace jccl {
   
ChunkDescDB::ChunkDescDB (): descs() {
    ;
}



ChunkDescDB::~ChunkDescDB() {
    ;
}


ChunkDescPtr ChunkDescDB::getChunkDesc (const std::string& _token) {
    for (unsigned int i = 0; i < descs.size(); i++)
        if (!vjstrcasecmp (descs[i]->token, _token))
            return descs[i];
    return ChunkDescPtr(NULL);
}



bool ChunkDescDB::insert (ChunkDescPtr d) {
    for (unsigned int i = 0; i < descs.size(); i++)
        if (!vjstrcasecmp (descs[i]->token, d->token)) {
            if (*descs[i] != *d) {
                vprDEBUG (vprDBG_ALL,vprDBG_CRITICAL_LVL) 
                    <<  clrOutNORM(clrRED, "ERROR:") 
                    << " redefinition of ChunkDesc ("
                    << d->name.c_str() << ") not allowed:\n  Original Desc: \n"
                    << *descs[i] << "\n  New Desc: \n" << *d
                    << "\n (multiple definitions must be identical)\n"
                    << vprDEBUG_FLUSH;
                vprASSERT (false);
                return false;
            }
            return true;
        }
    descs.push_back(d);
    return true;
}



void ChunkDescDB::insert (ChunkDescDB* db) {
    iterator begin = db->descs.begin();
    while (begin != db->descs.end()) {
        insert (*begin);//(new ChunkDesc(**begin));
        begin++;
    }
}



bool ChunkDescDB::remove (const std::string& tok) {

    iterator cur_desc = descs.begin();
    while (cur_desc != descs.end()) {
        if (!vjstrcasecmp ((*cur_desc)->token, tok)) {
            /// delete(*begin);     XXX:
            cur_desc = descs.erase(cur_desc);
            return true;
        }
        else
            cur_desc++;
    }
    return false;
}



void ChunkDescDB::removeAll () {
    descs.clear();
}



int ChunkDescDB::size () const {
    return descs.size();
}



std::ostream& operator << (std::ostream& out, const ChunkDescDB& self) {
    ConfigIO::instance()->writeChunkDescDB (out, self);
    return out;
}



std::istream& operator >> (std::istream& in, ChunkDescDB& self) {
    ConfigIO::instance()->readChunkDescDB (in, self);
    return in;
}



bool ChunkDescDB::load (const std::string& filename, const std::string& parentfile) {
    std::string fname = demangleFileName (filename, parentfile);
    bool retval = ConfigIO::instance()->readChunkDescDB (fname, *this);

    return retval;
}



bool ChunkDescDB::save (const char *file_name) {

    vprDEBUG(jcclDBG_CONFIG,3) 
        << "ChunkDescDB::save(): saving file " << file_name 
        << " -- " << vprDEBUG_FLUSH;    
    bool retval = ConfigIO::instance()->writeChunkDescDB (file_name, *this);
    if (retval) {
        vprDEBUG(jcclDBG_CONFIG,3)
            << " finished.\n" << vprDEBUG_FLUSH;
    }
    else {
        vprDEBUG(vprDBG_ERROR,1)
            << clrOutNORM(clrRED, "ERROR:") << " ChunkDescDB::save() - "
            << "Unable to open file '"
            << file_name << "'\n" << vprDEBUG_FLUSH;
    }
    return retval;
}


}; // namespace jccl
