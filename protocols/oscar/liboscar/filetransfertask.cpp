// filetransfertask.cpp

// Copyright (C)  2006

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301  USA

#include "filetransfertask.h"

#include <krandom.h>
#include <qstring.h>
#include <kdebug.h>
#include "buffer.h"
#include "connection.h"
#include "transfer.h"
#include "oscarutils.h"
#include <typeinfo>

FileTransferTask::FileTransferTask( Task* parent )
	:Task( parent ), m_action( Receive )
{
	kWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "uh, we don't actually support receiving yet" << endl;
}

FileTransferTask::FileTransferTask( Task* parent, const QString& contact, const QString &fileName )
	:Task( parent ), m_action( Send ), m_localFile( fileName ), m_contact( contact )
{
}

void FileTransferTask::onGo()
{
	if ( m_action == Receive )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "this isn't a send task" << endl;
		return;
	}
	if ( ( ! m_localFile.exists() ) || m_contact.isEmpty() )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "invalid vars" << endl;
		return;
	}

	sendFile();
}

bool FileTransferTask::take( Transfer* transfer )
{
	Q_UNUSED(transfer);
	return false;
}

bool FileTransferTask::take( int type, QByteArray cookie )
{
	if ( cookie != m_cookie )
		return false;

	//ooh, ooh, something happened!
	switch( type )
	{
	 case 0: //TODO: direct transfer ain't good enough
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "we don't handle requests yet!" << endl;
		break;
	 case 1: //TODO: delete self
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "we don't handle cancels yet!" << endl;
		break;
	 case 2: //TODO: tell user
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "we don't handle accepts yet!" << endl;
		break;
	 default:
		kWarning(OSCAR_RAW_DEBUG) << k_funcinfo << "bad request type: " << type << endl;
	}
	return true;
}

void FileTransferTask::sendFile()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << endl;
	//TODO: listen for connections

	//set up transfer
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0004, 0x0006, 0x0000, client()->snacSequence() };
	//this part prolly needs more organisation
	Buffer* b = new Buffer;
	//we get to make up an icbm cookie!
	DWORD cookie1 = KRandom::random();
	DWORD cookie2 = KRandom::random();
	b->addDWord( cookie1 );
	b->addDWord( cookie2 );
	//save the cookie for later
	m_cookie = b->buffer();
	//channel id
	b->addWord( 2 );
	//buddy info
	b->addByte( m_contact.length() );
	b->addString( m_contact.toLatin1() );

	//now create the rendezvous tlv
	b->addTLV( makeRendezvousRequest( m_cookie ) );

	//we're done, send it off!
	Transfer* t = createTransfer( f, s, b );
	send( t );
}

//TODO: test this
TLV FileTransferTask::makeRendezvousRequest( QByteArray cookie )
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "cookie size: " << cookie.size() << endl;
	Buffer rbuf;
	rbuf.addWord(0); //request type
	//copy the cookie
	rbuf.addString( cookie );

	rbuf.addGuid( oscar_caps[CAP_SENDFILE] );
	//now begins a bunch of tlvs
	char v1[] = { 0, 1 };
	rbuf.addTLV( 0xa, 2, v1 ); //request #
	rbuf.addTLV( 0xf, 0, 0 ); //unknown
	//rbuf.addTLV( 0xe, 2, "en" ); //language
	char v2[] = { 127, 0, 0, 1 };
	rbuf.addTLV( 0x2, 4, v2 ); //our ip FIXME
	char v3[] = { 0x80, 0xff, 0xff, 0xfe };
	rbuf.addTLV( 0x16, 4, v3 ); //ip check
	rbuf.addTLV( 0x3, 4, v2 ); //our ip FIXME
	char v4[] = { 0x14, 0x46 };
	rbuf.addTLV( 0x5, 2, v4 ); //port FIXME: guessed 5190, is that always true?
	char v5[] = { 0xeb, 0xb9 };
	rbuf.addTLV( 0x17, 2, v5 ); //port check

	Buffer fbuf; 
	fbuf.addWord( 1 ); //multiple file flag (we only support 1 right now)
	fbuf.addWord( 1 ); //file count
	fbuf.addDWord( m_localFile.size() ); //file count (FIXME: are we sure it's nonzero?)
	fbuf.addString( m_localFile.fileName().toLatin1() ); //name
	fbuf.addByte( 0 ); //make sure the name's null-terminated

	rbuf.addTLV( 0x2711, fbuf.length(), fbuf.buffer() );

	return TLV( 0x5, rbuf.length(), rbuf );
}

#include "filetransfertask.moc"


