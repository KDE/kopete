/*
   Copyright (c) 2002 Jason Keirstead <jason@keirstead.org> 
   Based on kio_sql.h (c) 2000 Praduroux Alessandro <pradu@thekompany.com>
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
 
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/     
#ifndef KIO_KOPETE_H
#define KIO_KOPETE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kio/slavebase.h>
#include <kio/global.h>
#include <qstring.h>

class KopeteProtocol : public KIO::SlaveBase {

public:
	KopeteProtocol( const QCString &pool, const QCString &app);
	

	void get( const KURL& url );
	void put( const KURL& url, int perms, bool overWrite, bool resume );
	void copy( const KURL& src, const KURL& dest, int perms, bool overWrite);
	void listDir( const KURL& url );
	void stat( const KURL& url );
private:
	void debug(QString msg);
	void createUDSEntry(QString path, QString filename, KIO::UDSEntry &);
	void createUDSEntry(int fileType, QString filename, KIO::UDSEntry &);
	void fillList( KIO::UDSEntry &e, QString methodName, const QString &arg0 = QString::null );
};

#endif

