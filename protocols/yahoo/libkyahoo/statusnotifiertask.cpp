/*
    Kopete Yahoo Protocol
    Handles several lists such as buddylist, ignorelist and so on

    Copyright (c) 2005 André Duffeck <andre.duffeck@kdemail.net>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <qstring.h>

#include "statusnotifiertask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>
#include <qstringlist.h>

StatusNotifierTask::StatusNotifierTask(Task* parent) : Task(parent)
{
	kdDebug(14180) << k_funcinfo << endl;
}

StatusNotifierTask::~StatusNotifierTask()
{

}

bool StatusNotifierTask::take( Transfer* transfer )
{
	kdDebug(14180) << k_funcinfo << endl;
	
	if ( !forMe( transfer ) )
		return false;
	
	parseStatus( transfer );	

	return true;
}

bool StatusNotifierTask::forMe( Transfer* transfer ) const
{
	kdDebug(14180) << k_funcinfo << endl;
	YMSGTransfer *t = 0L;
	t = dynamic_cast<YMSGTransfer*>(transfer);
	if (!t)
		return false;


	if ( t->service() == Yahoo::ServiceLogon ||
		t->service() == Yahoo::ServiceLogoff ||
		t->service() == Yahoo::ServiceIsAway ||
		t->service() == Yahoo::ServiceIsBack ||
		t->service() == Yahoo::ServiceGameLogon ||
		t->service() == Yahoo::ServiceGameLogoff ||
		t->service() == Yahoo::ServiceIdAct ||
		t->service() == Yahoo::ServiceIddeAct ||
		t->service() == Yahoo::ServiceStatus )
		return true;
	else
		return false;
}

void StatusNotifierTask::parseStatus( Transfer* transfer )
{
	kdDebug(14180) << k_funcinfo << endl;
	YMSGTransfer *t = 0L;
	t = dynamic_cast<YMSGTransfer*>(transfer);
	if (!t)
		return;

	QStringList nicks;	/* key = 7  */
	QStringList states;	/* key = 10  */
	QStringList session;	/* key = 11  */
	QStringList flags;	/* key = 13  */
	QStringList msgs;	/* key = 19  */
	QStringList aways;	/* key = 47  */
	QStringList mobile;	/* key = 60  */
	QStringList idles;	/* key = 137  */
	QString customError;	/* key = 16  */

	customError = t->firstParam( "16" );
	if( !customError.isEmpty() )
		emit error( customError );

	nicks = t->paramList( "7" );
	states = t->paramList( "10" );
	flags = t->paramList( "13" );
	msgs = t->paramList( "19" );
	aways = t->paramList( "47" );
	idles = t->paramList( "137" );

	for( uint i = 0; i < nicks.size(); ++i )
	{
		kdDebug(14180) << k_funcinfo << QString("Setting status of %1 to %2 away: %3 msg: %4").arg(nicks[i]).arg(states[i].toInt())
			.arg(aways[i].toInt()).arg( msgs[i] ) << endl; 
		if( t->service() == Yahoo::ServiceLogoff || flags[i] == 0 )
			emit statusChanged( nicks[i], Yahoo::StatusOffline, QString::null, 0 );
		else
			emit statusChanged( nicks[i], states[i].toInt(), msgs[i], aways[i].toInt() );
	}
}

#include "statusnotifiertask.moc"
