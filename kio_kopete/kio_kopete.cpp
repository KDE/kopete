/*
   This file is part of the KDB libraries
   Copyright (c) 2000 Praduroux Alessandro <pradu@thekompany.com>
 
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
#include "kio_kopete.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <kinstance.h>
#include <kdebug.h>
#include <klocale.h>
#include <dcopclient.h>


using namespace KIO;

extern "C" {
	int kdemain( int argc, char **argv ) {
		KInstance instance( "kio_kopete" );

		KopeteProtocol slave(argv[2], argv[3]);
		slave.dispatchLoop();

		return 0;
	}
}
 

KopeteProtocol::KopeteProtocol( const QCString &pool, const QCString &app):SlaveBase( "kopete", pool, app ) {
	
}

void KopeteProtocol::get( const KURL& url ) {

	//This is the function that will return file data
	//Output garbage for now
	
	mimeType("text/html");
	QCString output;
	output.sprintf("me\n");
	data(output);
	finished();
}
	
	 
void KopeteProtocol::listDir(const KURL & url) {
	
	UDSEntry entry;
	
	QString directory = url.directory();
	QString filename = url.fileName();

	if( directory == "/" && filename.isNull() ) {
		//At root, list all the contacts will file transfer capabilities
		fillList( entry, "fileTransferContacts()"); 
	} else if ( directory == "/" ) {
		/*
		 * FIXME We need check here to make sure contact exists before calling
		 */
		//We are at a contact, list his protocols
		fillList( entry, "contactFileProtocols(QString)", filename );
	} else {
		//This is either a file or an invalid path
	}
	
	listEntry(entry, true);
	finished();
	
}
 
//This function calls the DCOP functions to get directory data from Kopete
void KopeteProtocol::fillList( KIO::UDSEntry &e, QString methodName, const QString &arg0 ) {
		
	DCOPClient *client = new DCOPClient();
	QByteArray data, replyData;
	QCString replyType;
	QDataStream arg(data, IO_WriteOnly);
 
	arg << arg0;
 
	client->attach();
	QCString appId = client->registerAs("kio_kopete");

	if (client->call("kopete", "KopeteIface", methodName.latin1(), data, replyType, replyData)) {
		QDataStream reply(replyData, IO_ReadOnly);
		if (!replyData.isNull() && !replyData.isEmpty() && replyType == "QStringList") {
			QStringList result;
			reply >> result;
			for ( QStringList::Iterator it = result.begin(); it != result.end(); ++it ) {
				createUDSEntry( 2, *it, e );
				listEntry(e, false);
			}
		} 
	}
	client->detach();
	
	delete client;
}

//Get file information (?)
void KopeteProtocol::stat( const KURL& url ) {
	
	UDSEntry entry;

	createUDSEntry(url.directory(), url.fileName(), entry);

	statEntry( entry );
	finished();
}

void KopeteProtocol::createUDSEntry( QString path, QString fileName, KIO::UDSEntry &entry ) {
	QString full = path + "/" + fileName;
	createUDSEntry(2, fileName, entry);
}

void KopeteProtocol::createUDSEntry(int size, QString fileName, KIO::UDSEntry &entry) {

	//Size(contact) will be the # of protocols under the contact
	//Size(protocol) will be 0
	
	UDSAtom atom;

	entry.clear();
	
	atom.m_uds = KIO::UDS_NAME;
	if (fileName.isEmpty())
		atom.m_str = "/";
	else
		atom.m_str = fileName;
	entry.append( atom );
	
	//All our files are directories for now
	atom.m_uds = KIO::UDS_FILE_TYPE;
	atom.m_long = S_IFDIR;
        entry.append( atom );

	atom.m_uds = KIO::UDS_MIME_TYPE;
	atom.m_str = "inode/directory";
	entry.append( atom );
	
	atom.m_uds = UDS_ACCESS;
	atom.m_long = S_IRWXU | S_IRWXG | S_IRWXO;
	entry.append( atom );
}


