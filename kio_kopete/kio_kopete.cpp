/*
   Copyright (c) 2002 Jason Keirstead <jason@keirstead.org>
   Partially based on kio_sql.cpp (c) 2000 Praduroux Alessandro <pradu@thekompany.com>
 
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
#include <kfileitem.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <fstream>
 
using namespace KIO;
using namespace std;

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

	/*
	 * This is the function that will return file data,
	 * for copying a file from kopete. Output nothing for now
	 */
	
	mimeType("text/html");
	QCString output;
	output.sprintf("\n");
	data(output);
	finished();
}

//Called to copy a file to a contact (send a file)
void KopeteProtocol::put( const KURL& dest_url, int perms, bool overWrite, bool resume ) {

	int result;
	KFileItem *destFile = new KFileItem( KFileItem::Unknown, KFileItem::Unknown, dest_url );
	
	QByteArray data, replyData;
	QCString replyType;
	QDataStream arg(data, IO_WriteOnly);
	
	ofstream inout("/tmp/debug.out", ios::ate);
	
	//Create the DCOP Client
	DCOPClient *client = new DCOPClient();
	
	//Register with DCOP
	client->attach();
	QCString appId = client->registerAs("kio_kopete");
	
	//Pass contact, file, and new file name to function
	//arg << [contact] << [filename] << [new file name] << [filesize];
	arg << dest_url.directory() << "/tmp/kopete_kio_test" << dest_url.fileName() << destFile->size();
	
	//Create the FIFO for output if it doesn't exist
	mkfifo("/tmp/kopete_kio_test", 0600);
	
	if (client->call("kopete", "KopeteIface", "sendFile(QString &, KURL &, QString &, unsigned)", data, replyType, replyData)) {
		
		//Open the FIFO
		FILE *fp = fopen("/tmp/kopete_kio_test","wb");
		
		//Write the file
		do {
			QByteArray buffer;
			dataReq(); //Request Data from KIO
			result = readData( buffer ); //Populate buffer
			if (result > 0)
				fwrite(buffer.data(), buffer.size(), 1, fp); //Write to FIFO
				
		} while ( result > 0 );
		
		//Close the FIFO
		fclose(fp);
	
	} else {
		inout << "Couldn't connect to Kopete" << endl;
		//Couldn't connect to kopete!
	}
	
	client->detach(); //Detach from DCOP
	
	delete client;
	
	finished(); //Signal that we are finished
}

//Unused (?)
void KopeteProtocol::copy( const KURL& src, const KURL& dest, int perms, bool overWrite) {
	
	finished(); //Signal that we are finished
}

//List the contents of a directory	 
void KopeteProtocol::listDir(const KURL & url) {
	
	UDSEntry entry;
	
	QString directory = url.directory();
	QString filename = url.fileName();

	if( directory == "/" && filename.isNull() ) {
		//At root, list all the contacts with file transfer capabilities
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
	
	listEntry(entry, true); //Complete directory entry list
	
	finished(); //Signal that we are finished
	
}
 
//This function calls the DCOP functions to get directory data from Kopete
void KopeteProtocol::fillList( KIO::UDSEntry &e, QString methodName, const QString &arg0 ) {
		
	QByteArray data, replyData;
	QCString replyType;
	QDataStream arg(data, IO_WriteOnly);
	
	//Create the DCOP Client
	DCOPClient *client = new DCOPClient();
	
	//Read in arguments 
	arg << arg0;
 
	//Register with DCOP
	client->attach();
	QCString appId = client->registerAs("kio_kopete");

	//Call function
	if (client->call("kopete", "KopeteIface", methodName.latin1(), data, replyType, replyData)) {
		QDataStream reply(replyData, IO_ReadOnly);
		if (!replyData.isNull() && !replyData.isEmpty() && replyType == "QStringList") {
			//Read in results
			QStringList result;
			reply >> result;
			for ( QStringList::Iterator it = result.begin(); it != result.end(); ++it ) {
				
				//Add directory entries
				createUDSEntry( 2, *it, e );
				listEntry(e, false);
			}
		} 
	}
	
	//Detach from DCOP
	client->detach();
	
	delete client;
}

//Get file information
void KopeteProtocol::stat( const KURL& url ) {
	
	UDSEntry entry;

	createUDSEntry(url.directory(), url.fileName(), entry);

	statEntry( entry );
	finished();
}

//Overload for createUDSEntry
void KopeteProtocol::createUDSEntry( QString path, QString fileName, KIO::UDSEntry &entry ) {
	QString full = path + "/" + fileName;
	createUDSEntry(2, fileName, entry);
}

//Creates a UDS entry for the list of files
void KopeteProtocol::createUDSEntry(int size, QString fileName, KIO::UDSEntry &entry) {

	UDSAtom atom;
	
	entry.clear(); //Clear the existing attributes, if any
		
	//Populate filename
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

	//Mime type is directory
	atom.m_uds = KIO::UDS_MIME_TYPE;
	atom.m_str = "inode/directory";
	entry.append( atom );
	
	//Owner read / execute only 
	atom.m_uds = UDS_ACCESS;
	atom.m_long = 0600;
	entry.append( atom );
	
	/*
	 * Size(contact) will be the # of protocols under the contact
	 * Size(protocol) will be 0
	 */
}



