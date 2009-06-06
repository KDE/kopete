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
		ICQNotesInfo notesInfo;
		ICQShortInfo shortInfo;
		ICQInterestInfo interestInfo;
		ICQOrgAffInfo orgAffInfo;
		
		setTransfer( transfer );
		TLV tlv1 = transfer->buffer()->getTLV();
		Buffer* buffer = new Buffer( tlv1.data, tlv1.length );
		
		//FIXME this is silly. parseInitialData should take care of this for me.
		buffer->skipBytes( 12 );
		
		const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
		if ( !st )
			return false;
		
		Oscar::DWORD seq = st->snacRequest();
		QString contactId = m_contactSequenceMap[seq];
		
		switch ( requestSubType() )
		{
		case 0x00C8:  //basic user info
			kDebug( OSCAR_RAW_DEBUG ) << "Received basic info";
			genInfo.setSequenceNumber( seq );
			genInfo.fill( buffer );
			m_genInfoMap[seq] = genInfo;
			break;
		case 0x00D2:  //work user info
			kDebug( OSCAR_RAW_DEBUG ) << "Received work info";
			workInfo.setSequenceNumber( seq );
			workInfo.fill( buffer );
			m_workInfoMap[seq] = workInfo;
			break;
		case 0x00DC:  //more user info
			kDebug( OSCAR_RAW_DEBUG ) << "Received more info";
			moreInfo.setSequenceNumber( seq );
			moreInfo.fill( buffer );
			m_moreInfoMap[seq] = moreInfo;
			break;
		case 0x00E6:  //notes user info
			kDebug( OSCAR_RAW_DEBUG ) << "Received notes info";
			notesInfo.setSequenceNumber( seq );
			notesInfo.fill( buffer );
			m_notesInfoMap[seq] = notesInfo;
			break;
		case 0x00EB:  //email user info
			kDebug( OSCAR_RAW_DEBUG ) << "Received email info";
			emailInfo.setSequenceNumber( seq );
			emailInfo.fill( buffer );
			m_emailInfoMap[seq] = emailInfo;
			break;
		case 0x00F0:  //interests user info
			kDebug( OSCAR_RAW_DEBUG ) << "Received interest info";
			interestInfo.setSequenceNumber( seq );
			interestInfo.fill( buffer );
			m_interestInfoMap[seq] = interestInfo;
			break;
		case 0x00FA:  //affliations user info
			kDebug( OSCAR_RAW_DEBUG ) << "Received organization & affliation info";
			orgAffInfo.setSequenceNumber( seq );
			orgAffInfo.fill( buffer );
			m_orgAffInfoMap[seq] = orgAffInfo;
			//affliations seems to be the last info we get, so be hacky and only emit the signal once
			emit receivedInfoFor( contactId, Long );
			break;
		case 0x0104:
			kDebug( OSCAR_RAW_DEBUG ) << "Received short user info";
			shortInfo.setSequenceNumber( seq );
			shortInfo.fill( buffer );
			m_shortInfoMap[seq] = shortInfo;
			break;
		case 0x010E:  //homepage category user info
			kDebug( OSCAR_RAW_DEBUG ) << "Got homepage category info, but we don't support it yet";
			break;
		default:
			break;
		}
		
		
		if ( m_type == Short )
			emit receivedInfoFor( contactId, Short );

		setTransfer( 0 );
		delete buffer;
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
		kDebug(OSCAR_RAW_DEBUG) << "Requesting full user info for " << m_userToRequestFor;
	}
	else
	{
		setRequestSubType( 0x04BA );
		kDebug(OSCAR_RAW_DEBUG) << "Requesting short user info for " << m_userToRequestFor;
	}
	
	setSequence( client()->snacSequence() );
	setRequestType( 0x07D0 );
	b.addLEDWord( m_userToRequestFor.toULong() );
	sendBuf = addInitialData( &b );
	
	FLAP f = { 0x02, 0, 0 };
	Oscar::SNAC s = { 0x0015, 0x0002, 0, client()->snacSequence() };
	
	m_contactSequenceMap[s.id] = m_userToRequestFor;
	m_reverseContactMap[m_userToRequestFor] = s.id;
	
	Transfer* t = createTransfer( f, s, sendBuf );
	send( t );
}

ICQGeneralUserInfo ICQUserInfoRequestTask::generalInfoFor( const QString& contact )
{
	Oscar::DWORD seq = m_reverseContactMap[contact];
	return m_genInfoMap[seq];
}

ICQWorkUserInfo ICQUserInfoRequestTask::workInfoFor( const QString& contact )
{
	Oscar::DWORD seq = m_reverseContactMap[contact];
	return m_workInfoMap[seq];
}

ICQMoreUserInfo ICQUserInfoRequestTask::moreInfoFor( const QString& contact )
{
	Oscar::DWORD seq = m_reverseContactMap[contact];
	return m_moreInfoMap[seq];
}

ICQEmailInfo ICQUserInfoRequestTask::emailInfoFor( const QString& contact )
{
	Oscar::DWORD seq = m_reverseContactMap[contact];
	return m_emailInfoMap[seq];
}

ICQNotesInfo ICQUserInfoRequestTask::notesInfoFor( const QString& contact )
{
	Oscar::DWORD seq = m_reverseContactMap[contact];
	return m_notesInfoMap[seq];
}

ICQShortInfo ICQUserInfoRequestTask::shortInfoFor( const QString& contact )
{
	Oscar::DWORD seq = m_reverseContactMap[contact];
	return m_shortInfoMap[seq];
}

ICQInterestInfo ICQUserInfoRequestTask::interestInfoFor( const QString& contact )
{
	Oscar::DWORD seq = m_reverseContactMap[contact];
	return m_interestInfoMap[seq];
}

ICQOrgAffInfo ICQUserInfoRequestTask::orgAffInfoFor( const QString& contact )
{
	Oscar::DWORD seq = m_reverseContactMap[contact];
	return m_orgAffInfoMap[seq];
}

#include "icquserinfotask.moc"

//kate: indent-mode csands; tab-width 4; space-indent off; replace-tabs off;
