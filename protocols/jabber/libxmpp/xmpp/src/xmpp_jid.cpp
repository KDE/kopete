/*
 * jid.cpp - Jabber ID class
 * Copyright (C) 2001, 2002  Justin Karneges
 *                           Hideaki Omuro
 *                           Akito Nozaki
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include"xmpp_jid.h"

using namespace Jabber;

//! \brief Construct Jid object
//!
//! This function will construct a jabber id object.
Jid::Jid()
{
   QString a; // send an empty string to the set member
   set(a);

   // expansion
   //d = 0;
}

//! \brief Construct Jid object with a given QString id
//!
//! Overloaded constructor which will construct a jabber id object with the given QString id.
//! \param s - The jabber id QString (eg: joe@jabber.org)
Jid::Jid(const QString &s)
{
   set(s);

   // expansion
   //d = 0;
}

//! \brief Construct Jid object with a given string id
//!
//! Overloaded constructor which will construct a jabber id object with the given string id.
//! \param s - The jabber id string (eg: joe@jabber.org)
Jid::Jid(const char *s)
{
   set(s);

   // expansion
   //d = 0;
}

//! \brief Set Jid object's id
//!
//! Overloaded constructor which will construct a jabber id object with the given QString id.
//! \param s - The jabber id QString (eg: joe@jabber.org)
//! \note space inside Resources identifiers is supposed to be invalid, but we will allow it until further ado
void Jid::set(const QString &s)
{
   // assume the jid is invalid at the beginning
   v_isValid = false;

   // if the string is empty then return
   if(s.isEmpty())
      return;

   // declare variables
   int n, n2;
   QString tempuserhost, tempuser, temphost, tempresc;

   // initialize temporary user/host combo, user, host, resource variables
   tempuserhost = tempuser = temphost = tempresc = "";

   // first grab the username/host combo (everything before the /)
   n = s.find('/');
   if(n != -1)
      tempuserhost = s.mid(0, n);
   else
      tempuserhost = s;

   //-----------------------------------------------------------------------------------------------
   // from JEP-0029:
   // Node (username) identifiers are restricted to 256 bytes,
   // They may contain any Unicode character higher than #x20 with the exception of the following:
   //
   // #x22 ("), #x26 (&), #x27 ('), #x2F (/), #x3A (:), #x3C (<),
   // #x3E (>), #x40 (@), #x7F (del), #xFFFE (BOM), #xFFFF (BOM)
   //
   // Case is preserved, but comparisons will be made in case-normalized canonical form.
   //-----------------------------------------------------------------------------------------------

   // find/check username from the user/host combo
   n = tempuserhost.find('@');

   // it's not required so skip if we don't find the username
   if(n != -1)
   {
      tempuser = tempuserhost.mid(0, n);
      int i = tempuser.length();
      if(i > 256)
         return; // error (too long)

      // check for bad characters
      for(int j = 0; j < i; j++)
      {
         QChar chr = tempuser.at(j);
         switch(chr.unicode())
         {
            case 0x00:  case 0x01:  case 0x02:  case 0x03:
            case 0x04:  case 0x05:  case 0x06:  case 0x07:
            case 0x08:  case 0x09:  case 0x0A:  case 0x0B:
            case 0x0C:  case 0x0D:  case 0x0E:  case 0x0F:
            case 0x10:  case 0x11:  case 0x12:  case 0x13:
            case 0x14:  case 0x15:  case 0x16:  case 0x17:
            case 0x18:  case 0x19:  case 0x1A:  case 0x1B:
            case 0x1C:  case 0x1D:  case 0x1E:  case 0x1F:
            case 0x20:  case 0x22:  case 0x26:  case 0x27:
            case 0x2F:  case 0x3A:  case 0x3C:  case 0x3E:
            case 0x40:  case 0x7F:
            case 0xFFFE:         case 0xFFFF:
               return; // invalid char
            default:
               break;
         }
      }
   }

   //-----------------------------------------------------------------------------------------------
   // A domain identifier is a standard DNS hostname as specified in RFC952 and RFC1123.
   // (from RFC952)
   //   24 characters drawn from the alphabet (A-Z), digits (0-9), minus sign (-), and period (.)
   //   No distinction is made between upper and lower case.  The first character must be an alpha
   //   character.  The last character must not be a minus sign or period.
   //   Single character names or nicknames are not allowed.
   // (from RFC1123)
   //   the restriction on the first character is relaxed to allow either a letter or a digit.
   //   Host software MUST handle host names of up to 63 characters and SHOULD handle host names
   //   of up to 255 characters.
   //-----------------------------------------------------------------------------------------------
   // It is case-insensitive 7-bit ASCII (alphanumeric, minus, and period) and limited to 255 bytes.
   // It is the only REQUIRED component of a JID.

   // we will add 1 to the location of the @ to find the host.
   // (if @ wasn't found then the find returns -1, and we will increment to 0)
   n = tempuserhost.find('@') + 1;
   temphost = tempuserhost.mid(n);

   // get and check the length of the host name
   n = temphost.length();
   if(n < 2)
      return; //error (length must be at least 2)
   else if(n > 255)
      return; //error (length must be less than 255)

   // verify that the host is all host-valid ascii
   for(n2 = 0; n2 < n; n2++)
   {
      QChar chr;
      chr = temphost.at(n2);

      // validate each character
      if((chr >= 'A' && chr <= 'Z')
      || (chr >= 'a' && chr <= 'z')
      || (chr >= '0' && chr <= '9')
      || (chr == '-' || chr == '.'))
         continue;

      return; // error
   }

   //-----------------------------------------------------------------------------------------------
   // Resources identifiers are case-sensitive and are limited to 256 bytes. They may include any
   // Unicode character greater than #x20, except #xFFFE and #xFFFF.
   //-----------------------------------------------------------------------------------------------
   // NOTE: space is supposed to be invalid but we will allow it until further ado

   // resource
   n = s.find('/');
   if(n != -1)
   {
      tempresc = s.mid(n + 1);
      if(tempresc.length() > 256)
         return; //error (too long)

      // verify that the resource is all resource-valid unicode
      n = tempresc.length();
      for(n2 = 0; n2 < n; n2++)
      {
         QChar chr;
         chr = tempresc.at(n2);

         // validate each character
         if(chr.unicode() >= 0x20 && chr.unicode() < 0xFFFE)
            continue;

         return; // error
      }
   }

   // everything was good!!!
   // set all variables and exit
   v_user = tempuser;
   v_host = temphost;
   v_userHost = tempuserhost;
   v_resource = tempresc;

   v_isValid = true;

   buildJid();
}

//! \brief set the jid's resource
//!
//!
void Jid::setResource(const QString &_resource)
{
   v_resource = _resource;
   buildJid();
}

//! \brief return jid with modified resource
//!
//!
Jid Jid::withResource(const QString &_resource) const
{
   Jid newJid = *this;
   newJid.setResource(_resource);
   return newJid;
}

//! \brief build jid from components
//!
//!
void Jid::buildJid()
{
   realJid = "";
   if(!v_user.isEmpty())
   {
      realJid += v_user;
      realJid += '@';
   }
   realJid += v_host;
   if(!v_resource.isEmpty())
   {
      realJid += '/';
      realJid += v_resource;
   }
}

//! \brief Compare jid
//!
//!
bool Jid::compare(const Jid &s, bool compareRes) const
{
   if(v_user.lower() != s.v_user.lower())
      return false;

   if(compareRes) {
      if(v_resource != s.v_resource)
         return false;
   }

   if(v_host.lower() != s.v_host.lower())
      return false;

   return true;
}

Jid::~Jid()
{
// if(d)
//    delete d;
}


bool Jid::isValid() const
{
   return v_isValid;
}

bool Jid::isEmpty() const
{
   return realJid.isEmpty();
}

const QString & Jid::user() const
{
   return v_user;
}

const QString & Jid::host() const
{
   return v_host;
}

const QString & Jid::resource() const
{
   return v_resource;
}

const QString & Jid::full() const
{
   return realJid;
}

const QString & Jid::userHost() const
{
   return v_userHost;
}

Jid & Jid::operator=(const QString &s)
{
   this->set(s);
   return *this;
}

Jid & Jid::operator=(const char *s)
{
   QString a(s);
   this->set(a);
   return *this;
}

