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
#include <jccl/Config/ConfigChunk.h>
#include <jccl/Config/ParseUtil.h>
#include <jccl/Util/Debug.h>
#include <vpr/Util/Assert.h>
#include <jccl/Config/ChunkFactory.h>
#include <jccl/Config/ChunkDesc.h>
#include <jccl/Config/ConfigTokens.h>
#include <jccl/Config/ConfigIO.h>

#include <stdlib.h>
#include <ctype.h>


namespace jccl {
   
/*static*/ const std::string ConfigChunk::embedded_separator("->");

ConfigChunk::ConfigChunk (): props(), type_as_varvalue(T_STRING) {
    validation = 1;
}



ConfigChunk::ConfigChunk (ChunkDescPtr d, bool use_defaults) :props(), type_as_varvalue(T_STRING) {
    validation = 1;
    associateDesc (d, use_defaults);
}


ConfigChunk::ConfigChunk (const ConfigChunk& c):props(), type_as_varvalue(T_STRING) {
    validation = 1;
    *this = c;
}


ConfigChunk::~ConfigChunk () {
    assertValid();

    validation = 0;

    for (unsigned int i = 0; i < props.size(); i++)
        delete (props[i]);

}

bool ConfigChunk::hasSeparator (const std::string &path) {
    return (path.find(embedded_separator) != path.npos);
}

std::string ConfigChunk::getRemainder (const std::string &path) {
    std::string::size_type i = path.find (embedded_separator);
    if (i == path.npos)
        return path;
    else 
        return path.substr (i + embedded_separator.length());
}

std::string ConfigChunk::getFirstNameComponent (const std::string& path) {
    std::string::size_type i = path.find (embedded_separator);
    if (i == path.npos)
        return path;
    else 
        return path.substr (0, i);
}

#ifdef JCCL_DEBUG
void ConfigChunk::assertValid () const {
    vprASSERT (validation == 1 && "Trying to use deleted config chunk");
    for (unsigned int i = 0; i < props.size(); i++)
        props[i]->assertValid();
}
#endif



void ConfigChunk::associateDesc (ChunkDescPtr d, bool use_defaults) {
    assertValid();
    ConfigChunkPtr ch;
    if (use_defaults)
        ch = d->getDefaultChunk();
    if (ch.get() != 0)
        *this = *ch;
    else {

        unsigned int i;

        desc = d;
        type_as_varvalue = desc->getToken();

        for (i = 0; i < props.size(); i++)
            delete (props[i]);

        props.clear();
        
        for (i = 0; i < desc->plist.size(); i++) {
            PropertyDesc* pd = desc->plist[i];
            Property* pr = new Property (pd);
            props.push_back (pr);
        }
    }
}



ConfigChunk& ConfigChunk::operator = (const ConfigChunk& c) {
    assertValid();
    c.assertValid();

    unsigned int i;
    if (this == &c)     // ack! same object!
        return *this;

    desc = c.desc;
    type_as_varvalue = c.type_as_varvalue;


    for (i = 0; i < props.size(); i++)
        delete (props[i]);

    props.clear();

    for (i = 0; i < c.props.size(); i++) {
        props.push_back (new Property(*(c.props[i])));
    }
    return *this;
}



//: tests for equality of two ConfigChunks
bool ConfigChunk::operator== (const ConfigChunk& c) const {
    assertValid();
    c.assertValid();

    // equality operator - this makes a couple of assumptions:
    // 1. the descs must be the _same_object_, not just equal.
    // 2. the properties will be in the _same_order_.  This is
    //    reasonable if 1. is true.

    if (desc.get() != c.desc.get())
        return false;
    if (props.size() != c.props.size()) // probably redundant
        return false;
    for (unsigned int i = 0; i < props.size(); i++) {
        if (*(props[i]) != *(c.props[i]))
            return false;
    }
    return true;
}



//: Compares two ConfigChunks based on their instance names
bool ConfigChunk::operator< (const ConfigChunk& c) const {
    assertValid();

    std::string s1 = getProperty ("name");
    std::string s2 = c.getProperty ("name");
    return s1 < s2;
}



// used for dependency resolution
ConfigChunkPtr ConfigChunk::getEmbeddedChunk (const std::string &path) {
    std::string propname, chunkname, subpath;
    Property* prop;
    int i;
    ConfigChunkPtr ch(this);// *ch = this;
    //ConfigChunk *ch2, *ch3;
    ConfigChunkPtr ch2, ch3;
  
    if (vjstrcasecmp (ch->getName(), path /*getFirstNameComponent (path)*/)) {
        return ConfigChunkPtr(0);
    }
    else {
        if (!hasSeparator(path))
            return ch;
        else {
            subpath = getRemainder(path); // strip chunkname
            propname = getFirstNameComponent(path);
            subpath = getRemainder(path); // strip propname
            chunkname = getFirstNameComponent (path);
            prop = getPropertyPtrFromName(propname);
            if (prop) {
                for (i = 0; i < prop->getNum(); i++) {
                    ch2 = (ConfigChunkPtr)prop->getValue(i);
                    if (ch2.get() != 0) {
                        ch3 = ch2->getEmbeddedChunk(path);
                        if (ch3.get() != 0)
                            return ch3;
                    }
                }
            }
            else {
                // this next bit is insurance against some of my 
                // own most likely mistakes
                prop = getPropertyPtrFromToken(propname);
                if (prop) {
                    for (i = 0; i < prop->getNum(); i++) {
                        ch2 = (ConfigChunkPtr)prop->getValue(i);
                        if (ch2.get() != 0) {
                            ch3 = ch2->getEmbeddedChunk(path);
                            if (ch3.get() != 0)
                                return ch3;
                        }
                    }
                }
            }
            return ConfigChunkPtr();
        }
    }
}



//: Return a list of chunk names dependant upon this one
// This is used to sort a db by dependancy.
std::vector<std::string> ConfigChunk::getChunkPtrDependencies() const
{
    assertValid();

   std::string chunkname;
   std::vector<std::string> dep_list;     // Create return vector
   unsigned int i;
   int j;

   //cout << "Dependency test for " << getProperty ("name") << endl;
   for (i=0;i<props.size();i++)                       // For each property
   {
      if (props[i]->getType() == T_CHUNK)             // If it is a chunk ptr
      {
         for (j=0;j<props[i]->getNum();j++)           // For each property
         {
            VarValue prop_var_val = props[i]->getValue(j);
            chunkname = static_cast<std::string>(prop_var_val);
            if (!(chunkname == ""))
            {
               dep_list.push_back(chunkname);
            }
         }
      }
      else if (props[i]->getType() == T_EMBEDDEDCHUNK)
      {
         std::vector<std::string> child_deps;
         for (j = 0; j < props[i]->getNum(); j++)
         {
            // XXX: if we ever have cyclic dependencies, we're in trouble
            child_deps = ((ConfigChunkPtr)props[i]->getValue(j))->getChunkPtrDependencies();
            dep_list.insert (dep_list.end(), child_deps.begin(), child_deps.end());
         }
      }
   }
   return dep_list;      // Return the list
}



Property* ConfigChunk::getPropertyPtrFromName (const std::string& property) const {
    assertValid();

    for (unsigned int i = 0; i < props.size(); i++) {
        if (!vjstrcasecmp (props[i]->getName(), property))
            return props[i];
    }
    return NULL;
}



Property* ConfigChunk::getPropertyPtrFromToken (const std::string& token) const {
    assertValid();

    for (unsigned int i = 0; i < props.size(); i++) {
        if (!vjstrcasecmp(props[i]->getToken(), token))
            return props[i];
    }
    return NULL;
}



//: Return all the values for a given property
// This is just a simple helper function
//! NOTE: The vector has COPIES of the var values.
// cj - this is bad implementation... bad...
std::vector<VarValue*> ConfigChunk::getAllProperties(const std::string& prop_name) const
{
    assertValid();

    std::vector<VarValue*> ret_val;
    Property* p = getPropertyPtrFromToken(prop_name);
    if (p) {
        int num_properties = p->getNum();
        for(int i=0;i<num_properties;i++) {
            VarValue* new_var_val = new VarValue(p->getValue(i));
            ret_val.push_back(new_var_val);
        }
    }

    return ret_val;
}



std::ostream& operator << (std::ostream& out, const ConfigChunk& self) {
    self.assertValid();

    ConfigIO::instance()->writeConfigChunk (out, self);
    return out;
}



int ConfigChunk::getNum (const std::string& property_token) const {
    assertValid();

    Property* p = getPropertyPtrFromToken (property_token);
    if (p)
        return p->getNum();
    else
        return 0;
}



//: Returns token of this ConfigChunk's ChunkDesc.
const std::string& ConfigChunk::getDescToken () const {
    return desc->getToken();
}



const VarValue& ConfigChunk::getType () const {
    assertValid();

    return type_as_varvalue;
}



const VarValue& ConfigChunk::getProperty (const std::string& property_token, int ind) const {
    assertValid();

    if (!vjstrcasecmp(property_token,type_TOKEN)) {
        return type_as_varvalue;
    }

    Property *p = getPropertyPtrFromToken (property_token);
    if (!p) {
        vprDEBUG(jcclDBG_CONFIG,2) << "getProperty(\"" << property_token.c_str() << "\") in chunk \""
                                << getProperty("Name") << "\" - no such property; returning T_INVALID\n" << vprDEBUG_FLUSH;
        return VarValue::getInvalidInstance();
    }
    return p->getValue (ind);
}


/* we're probably gonna need to overload set for each kind of
 * value.  That gets passed on to the VarValue assign...
 */

bool ConfigChunk::setProperty (const std::string& property, int val, int ind) {
    assertValid();

    Property *p;
    p = getPropertyPtrFromToken (property);
    if (!p)
        return false;
    return p->setValue (val, ind);
}

bool ConfigChunk::setProperty (const std::string& property, float val, int ind) {
    assertValid();

    Property *p;
    p = getPropertyPtrFromToken (property);
    if (!p)
        return false;
    return p->setValue (val, ind);
}

bool ConfigChunk::setProperty (const std::string& property, const std::string& val, int ind) {
    assertValid();

    Property *p;
    p = getPropertyPtrFromToken (property);
    if (!p)
        return false;
    return p->setValue (val, ind);
}

bool ConfigChunk::setProperty (const std::string& property, ConfigChunkPtr val, int ind) {
    assertValid();

    Property *p;
    p = getPropertyPtrFromToken (property);
    if (!p) {
        vprDEBUG (vprDBG_ERROR, 1) << "ConfigChunk.setProperty: no such property " << property.c_str()
                                 << "\n" << vprDEBUG_FLUSH;
        return false;
    }
    return p->setValue (val, ind);
}


bool ConfigChunk::addValue (const std::string& property, int val) {
    assertValid();

    Property *p;
    p = getPropertyPtrFromToken (property);
    if (p == NULL)
        return false;
    if (p->hasFixedNumberOfValues())
        return false;
    return setProperty (property, val, p->value.size());
}

bool ConfigChunk::addValue (const std::string& property, float val) {
    assertValid();

    Property *p;
    p = getPropertyPtrFromToken (property);
    if (p == NULL)
        return false;
    if (p->hasFixedNumberOfValues())
        return false;
    return setProperty (property, val, p->value.size());
}

bool ConfigChunk::addValue (const std::string& property, const std::string& val) {
    assertValid();

    Property *p;
    p = getPropertyPtrFromToken (property);
    if (p == NULL)
        return false;
    if (p->hasFixedNumberOfValues())
        return false;
    return setProperty (property, val, p->value.size());
}

bool ConfigChunk::addValue (const std::string& property, ConfigChunkPtr val) {
    assertValid();

    Property *p;
    p = getPropertyPtrFromToken (property);
    if (p == NULL)
        return false;
    if (p->hasFixedNumberOfValues())
        return false;
    return setProperty (property, val, p->value.size());
}


};
