/*
  Kopete Oscar Protocol
  icqtask.h - SNAC 0x15 parsing 

  Copyright (c) 2004 Matt Rogers <mattr@kde.org>

  Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

  *************************************************************************
  *                                                                       *
  * This library is free software; you can redistribute it and/or         *
  * modify it under the terms of the GNU Lesser General Public            *
  * License as published by the Free Software Foundation; either          *
  * version 2 of the License, or (at your option) any later version.      *
  *                                                                       *
  *************************************************************************
*/

#include "icquserinfotask.h"
#include <kdebug.h>
#include "connection.h"
#include "transfer.h"
#include "buffer.h"


ICQUserInfoRequestTask::ICQUserInfoRequestTask( Task* parent ) : ICQTask( parent )
{
	//by default, request short info. it saves bandwidth
	m_type = Short;
}


ICQUserInfoRequestTask::~ICQUserInfoRequestTask()
{
}


bool ICQUserInfoRequestTask::forMe( const Transfer* transfer ) const
{
	const SnacTransfer * st = dynamic_cast<const SnacTransfer*>( transfer );

	if ( !st )
		return false;

	if ( st->snacService() != 0x0015 || st->snacSubtype() != 0x0003 )
		return false;

	Buffer buf( *( st->buffer() ) );
	const_cast<ICQUserInfoRequestTask*>( this )->parseInitialData( buf );

	if ( requestType() == 0x07DA )
	{
		switch ( requestSubType() )
		{
		case 0x00C8:
		case 0x00D2:
		case 0x00DC:
		case 0x00E6:
		case 0x00EB:
		case 0x00F0:
		case 0x00FA:
		case 0x0104:
		case 0x010E:
			return true;
		default:
			return false;
		}
	}

	return false;
}

bool ICQUserInfoRequestTask::take( Transfer* transfer )
{
	if ( forMe( transfer ) )
	{
		ICQGeneralUserInfo genInfo;
		ICQWorkUserInfo workInfo;
		ICQMoreUserInfo moreInfo;
		ICQEmailInfo emailInfo;
		ICQShortInfo shortInfo;
		ICQInterestInfo interestInfo;
		
		setTransfer( transfer );
		TLV tlv1 = transfer->buffer()->getTLV();
		Buffer* buffer = new Buffer( tlv1.data, tlv1.length );
		
		//FIXME this is silly. parseInitialData should take care of this for me.
		buffer->skipBytes( 8 );
		WORD seq = buffer->getLEWord(); // request sequence number
		buffer->getLEWord(); // request data sub type
		QString contactId = m_contactSequenceMap[seq];
		
		switch ( requestSubType() )
		{
		case 0x00C8:  //basic user info
			kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Received basic info" << endl;
			genInfo.setSequenceNumber( seq );
			genInfo.fill( buffer );
			m_genInfoMap[seq] = genInfo;
			break;
		case 0x00D2:  //work user info
			kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Received work info" << endl;
			workInfo.setSequenceNumber( seq );
			workInfo.fill( buffer );
			m_workInfoMap[seq] = workInfo;
			break;
		case 0x00DC:  //more user info
			kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Received more info" << endl;
			moreInfo.setSequenceNumber( seq );
			moreInfo.fill( buffer );
			m_moreInfoMap[seq] = moreInfo;
			break;
		case 0x00E6:  //notes user info
			kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Got Notes info, but we don't support it yet" << endl;
			break;
		case 0x00EB:  //email user info
			kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Received email info" << endl;
			emailInfo.setSequenceNumber( seq );
			emailInfo.fill( buffer );
			m_emailInfoMap[seq] = emailInfo;
			break;
		case 0x00F0:  //interests user info
			kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Received interest info" << endl;
			interestInfo.setSequenceNumber( seq );
			interestInfo.fill( buffer );
			m_interestInfoMap[seq] = interestInfo;
			break;
		case 0x00FA:  //affliations user info
			//affliations seems to be the last info we get, so be hacky and only emit the signal once
			emit receivedInfoFor( contactId, Long );
			kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Got affliations info, but we don't support it yet" << endl;
			break;
		case 0x0104:
			kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Received short user info" << endl;
			shortInfo.setSequenceNumber( seq );
			shortInfo.fill( buffer );
			m_shortInfoMap[seq] = shortInfo;
			break;
		case 0x010E:  //homepage category user info
			kdDebug( OSCAR_RAW_DEBUG ) << k_funcinfo << "Got homepage category info, but we don't support it yet" << endl;
			break;
		default:
			break;
		}
		
		
		if ( m_type == Short )
			emit receivedInfoFor( contactId, Short );

		setTransfer( 0 );
		return true;
	}
	return false;
}

void ICQUserInfoRequestTask::onGo()
{
	if ( m_userToRequestFor.isNull() )
		return;
	
	Buffer* sendBuf = 0L;
	Buffer b;
	if ( m_type != Short )
	{
		setRequestSubType( 0x04D0 );
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Requesting full user info for " << m_userToRequestFor << endl;
	}
	else
	{
		setRequestSubType( 0x04BA );
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Requesting short user info for " << m_userToRequestFor << endl;
	}
	
	setSequence( client()->snacSequence() );
	setRequestType( 0x07D0 );
	b.addLEDWord( m_userToRequestFor.toULong() );
	sendBuf = addInitialData( &b );
	
	m_contactSequenceMap[sequence()] = m_userToRequestFor;
	m_reverseContactMap[m_userToRequestFor] = sequence();
	
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0015, 0x0002, 0, client()->snacSequence() };
	Transfer* t = createTransfer( f, s, sendBuf );
	send( t );
}

ICQGeneralUserInfo ICQUserInfoRequestTask::generalInfoFor( const QString& contact )
{
	int seq = m_reverseContactMap[contact];
	return m_genInfoMap[seq];
}

ICQWorkUserInfo ICQUserInfoRequestTask::workInfoFor( const QString& contact )
{
	int seq = m_reverseContactMap[contact];
	return m_workInfoMap[seq];
}

ICQMoreUserInfo ICQUserInfoRequestTask::moreInfoFor( const QString& contact )
{
	int seq = m_reverseContactMap[contact];
	return m_moreInfoMap[seq];
}

ICQEmailInfo ICQUserInfoRequestTask::emailInfoFor(const QString& contact )
{
	int seq = m_reverseContactMap[contact];
	return m_emailInfoMap[seq];
}

ICQShortInfo ICQUserInfoRequestTask::shortInfoFor( const QString& contact )
{
	int seq = m_reverseContactMap[contact];
	return m_shortInfoMap[seq];
}

ICQInterestInfo ICQUserInfoRequestTask::interestInfoFor( const QString& contact )
{
	int seq = m_reverseContactMap[contact];
	return m_interestInfoMap[seq];
}

QString ICQUserInfoRequestTask::notesInfoFor( const QString& contact )
{
	int seq = m_reverseContactMap[contact];
	return m_notesInfoMap[seq];
}


#include "icquserinfotask.moc"

//kate: indent-mode csands; tab-width 4; space-indent off; replace-tabs off;
