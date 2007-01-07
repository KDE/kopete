/*
    Kopete Oscar Protocol
    changevisibilitytask.cpp - Changes the visibility of the account via SSI

    Copyright (c) 2005 Matt Rogers <mattr@kde.org>

    Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "changevisibilitytask.h"

#include <qvaluelist.h>
#include <kdebug.h>
#include "buffer.h"
#include "client.h"
#include "connection.h"
#include "oscartypeclasses.h"
#include "oscartypes.h"
#include "oscarutils.h"
#include "ssimanager.h"
#include "transfer.h"


ChangeVisibilityTask::ChangeVisibilityTask(Task* parent): Task(parent)
{
	m_sequence = 0;
	m_visible = true;
}


ChangeVisibilityTask::~ChangeVisibilityTask()
{
}

void ChangeVisibilityTask::setVisible( bool visible )
{
	m_visible = visible;
}

bool ChangeVisibilityTask::forMe(const Transfer* transfer) const
{
	const SnacTransfer* st = dynamic_cast<const SnacTransfer*>( transfer );
	if ( !st )
		return false;
	
	SNAC s = st->snac(); //cheat
	if ( s.family == 0x0013 && s.subtype == 0x000E )
		return true;
	else
		return false;
}

bool ChangeVisibilityTask::take(Transfer* transfer)
{
	if ( forMe( transfer ) )
	{
		setTransfer( transfer );
		setSuccess( 0, QString::null );
		setTransfer( 0 );
		return true;
	}
	else
	{
		setError( 0, QString::null );
		return false;
	}
}

void ChangeVisibilityTask::onGo()
{
	SSIManager* manager = client()->ssiManager();
	Oscar::SSI item = manager->visibilityItem();
	if ( !item )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Didn't find a visibility item" << endl;
		setError( 0, QString::null );
		return;
	}
	
	Buffer c8tlv;
	BYTE visibleByte = m_visible ? 0x04 : 0x03;
	c8tlv.addByte( visibleByte );
	
	QValueList<Oscar::TLV> tList;
	tList.append( TLV( 0x00CA, c8tlv.length(), c8tlv.buffer() ) );
	
	Oscar::SSI newSSI(item);
	if ( Oscar::uptateTLVs( newSSI, tList ) == false )
	{
		kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Visibility didn't change, don't update" << endl;
		setSuccess( 0, QString::null );
		return;
	}
	
	//remove the old item and add the new item indicating the
	//change in visibility.
	manager->removeItem( item );
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "found visibility item. changing setting" << endl;
	manager->newItem( newSSI );
	sendEditStart();
	
	Buffer* b = new Buffer();
	FLAP f = { 0x02, 0, 0 };
	SNAC s = { 0x0013, 0x0009, 0x0000, client()->snacSequence() };
	m_sequence = s.id;
	b->addWord( 0 );
	b->addWord( newSSI.gid() );
	b->addWord( newSSI.bid() );
	b->addWord( newSSI.type() );
	b->addWord( newSSI.tlvListLength() );
	
	QValueList<TLV>::const_iterator it2 =  newSSI.tlvList().begin();
	QValueList<TLV>::const_iterator listEnd2 = newSSI.tlvList().end();
	for( ; it2 != listEnd2; ++it2 )
		b->addTLV( ( *it2 ) );
	
	kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Sending visibility update" << endl;
	Transfer* t = createTransfer( f, s, b );
	send( t );
	sendEditEnd();
}

void ChangeVisibilityTask::sendEditStart()
{
	SNAC editStartSnac = { 0x0013, 0x0011, 0x0000, client()->snacSequence() };
	FLAP editStart = { 0x02, 0, 0 };
	Buffer* emptyBuffer = new Buffer;
	Transfer* t1 = createTransfer( editStart, editStartSnac, emptyBuffer );
	send( t1 );
}

void ChangeVisibilityTask::sendEditEnd()
{
	SNAC editEndSnac = { 0x0013, 0x0012, 0x0000, client()->snacSequence() };
	FLAP editEnd = { 0x02, 0, 0 };
	Buffer* emptyBuffer = new Buffer;
	Transfer *t5 = createTransfer( editEnd, editEndSnac, emptyBuffer );
	send( t5 );
}

//kate: indent-mode csands; space-indent off; replace-tabs off; tab-width 4;
