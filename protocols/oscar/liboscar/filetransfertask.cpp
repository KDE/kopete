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

bool FileTransferTask::forMe( const Transfer* transfer )
{
	//TODO
/*	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
	if ( !st )
		return false;

	if ( st->snacRequest() != m_seq )
	{
		kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "sequences don't match" << endl;
		return false;
	}

	if ( st->snacService() == 0x0010 )
	{
		switch( st->snacSubtype() )
		{
		case 0x0003:
		case 0x0005:
		case 0x0007:
			return true;
			break;
		default:
			return false;
			break;
		}
	}*/

	return false;
}

bool FileTransferTask::take( Transfer* transfer )
{
	if ( !forMe( transfer ) )
		return false;

	//TODO
/*	SnacTransfer* st = dynamic_cast<SnacTransfer*>( transfer );
	if ( !st )
		return false;

	setTransfer( transfer );
	if ( st->snacSubtype() == 0x0003 )
		handleUploadResponse();
	else if ( st->snacSubtype() == 0x0005 )
		handleAIMBuddyIconResponse();
	else
		handleICQBuddyIconResponse();

	setSuccess( 0, QString::null );
	setTransfer( 0 );*/
	return true; //can't happen
}

void FileTransferTask::sendFile()
{
	kDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "pretend we sent a file :)" << endl;
	//TODO
/*	kDebug(OSCAR_RAW_DEBUG) << "icon length: " << m_iconLength << endl;
	FLAP f = { 0x02, 0, 0 };
	m_seq = client()->snacSequence();
	SNAC s = { 0x0010, 0x0002, 0x0000, m_seq };
	Buffer* b = new Buffer;
	b->addWord( 1 ); //gaim hard codes it, so will we
	b->addWord( m_iconLength );
	b->addString( m_icon );
	Transfer* t = createTransfer( f, s, b );
	send( t );*/
}

#include "filetransfertask.moc"


