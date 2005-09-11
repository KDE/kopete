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

#include "mailnotifiertask.h"
#include "transfer.h"
#include "ymsgtransfer.h"
#include "yahootypes.h"
#include "client.h"
#include <qstring.h>

MailNotifierTask::MailNotifierTask(Task* parent) : Task(parent)
{
	kdDebug(14180) << k_funcinfo << endl;
}

MailNotifierTask::~MailNotifierTask()
{

}

bool MailNotifierTask::take( Transfer* transfer )
{
	kdDebug(14180) << k_funcinfo << endl;
	
	if ( !forMe( transfer ) )
		return false;

	parseMail( transfer );

	return true;
}

bool MailNotifierTask::forMe( Transfer* transfer ) const
{
	kdDebug(14180) << k_funcinfo << endl;
	YMSGTransfer *t = 0L;
	t = dynamic_cast<YMSGTransfer*>(transfer);
	if (!t)
		return false;

	if ( t->service() == Yahoo::ServiceNewMail )
		return true;
	else
		return false;
}

void MailNotifierTask::parseMail( Transfer *transfer )
{
	kdDebug(14180) << k_funcinfo << endl;
	YMSGTransfer *t = 0L;
	t = dynamic_cast<YMSGTransfer*>(transfer);
	if (!t)
		return;

	QString count = t->firstParam( 9 );
	QString mail = t->firstParam( 42 );
	QString from = t->firstParam( 43 );
	QString subject = t->firstParam( 18 );

	if( !mail.isEmpty() && !from.isEmpty() && !subject.isEmpty() )
		emit mailNotify( QString::fromLatin1( "%1 <%2>").arg( from, mail ), subject, count.toInt() );
	else
		emit mailNotify( QString::null, QString::null, count.toInt());
}

#include "mailnotifiertask.moc"
