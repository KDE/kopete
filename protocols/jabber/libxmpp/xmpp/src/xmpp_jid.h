/*
 * jid.h - Jabber ID class
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

#ifndef JABBER_JID_H
#define JABBER_JID_H

#include<qstring.h>

namespace Jabber
{
   class Jid
   {
   public:
      Jid();
      Jid(const QString &);
      Jid(const char *);
      ~Jid();

      void set(const QString &);
      void setResource(const QString &);
      Jid withResource(const QString &) const;

      Jid & operator=(const QString &);
      Jid & operator=(const char *);

      bool isValid() const;
      bool isEmpty() const;
      bool compare(const Jid &, bool compareRes=true) const;

      const QString & user() const;
      const QString & host() const;
      const QString & resource() const;
      const QString & full() const;
      const QString & userHost() const;

   private:
      void buildJid();

      QString realJid;
      QString v_user, v_host, v_resource, v_userHost;
      bool v_isValid;

      //just in case we need to add anything later to this class
      class JidPrivate *d;
      //JidPrivate *d;
   };

}

#endif //JABBER_JID_H
