/*
    Kopete Oscar Protocol
    closeconnectiontask.h - Handles the closing of the connection to the server

    Copyright (c) 2004 Matt Rogers <mattr@kde.org>
    Copyright (c) 2007 Roman Jarosz <kedgedev@centrum.cz>

    Kopete (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "closeconnectiontask.h"

#include <qstring.h>
#include <QList>
#include <kdebug.h>
#include <klocale.h>
#include "connection.h"
#include "transfer.h"
#include "oscarutils.h"

using namespace Oscar;

CloseConnectionTask::CloseConnectionTask( Task* parent )
	: Task(parent)
{
}


CloseConnectionTask::~CloseConnectionTask()
{
}

bool CloseConnectionTask::take( Transfer* transfer )
{
	if ( forMe( transfer ) )
	{
		kDebug(OSCAR_RAW_DEBUG) << "RECV (DISCONNECT)";

		FlapTransfer* ft = dynamic_cast<FlapTransfer*> ( transfer );

		if ( !ft )
		{
			kDebug(OSCAR_RAW_DEBUG) 
				<< "Could not convert transfer object to type FlapTransfer!!"  << endl;
			return false;
		}

		QList<TLV> tlvList = ft->buffer()->getTLVList();

		TLV err = findTLV( tlvList, 0x0009 );
		if ( err )
		{
			Oscar::WORD errorNum = ( ( err.data[0] << 8 ) | err.data[1] );

			kDebug(OSCAR_RAW_DEBUG) << "found TLV(8) [ERROR] error= " << errorNum;

			Oscar::SNAC s = { 0, 0, 0, 0 };
			client()->fatalTaskError( s, errorNum );
			return true; //if there's an error, we'll need to disconnect anyways
		}

		setSuccess( 0, QString() );
		return true;
	}
	return false;
}

bool CloseConnectionTask::forMe( const Transfer* transfer ) const
{
	const FlapTransfer* ft = dynamic_cast<const FlapTransfer*> ( transfer );

	if (!ft)
		return false;

	if ( ft && ft->flapChannel() == 4 )
		return true;
	else
		return false;
}

void CloseConnectionTask::onGo()
{
	FLAP f = { 0x04, 0, 0 };
	
	Transfer* ft = createTransfer( f, new Buffer() );
	kDebug(OSCAR_RAW_DEBUG) << "Sending channel 0x04 close packet";
	send( ft );
	setSuccess( 0, QString() );
}

//kate: tab-width 4; indent-mode csands;
