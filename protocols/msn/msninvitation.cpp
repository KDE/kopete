/*
    msninvitation.cpp

    Copyright (c) 2003 by Olivier Goffart        <ogoffart @ kde.org>

    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include "msninvitation.h"
#include <stdlib.h>
#include <qregexp.h>

MSNInvitation::MSNInvitation(bool incoming, const QString &applicationID , const QString &applicationName)
{
	m_incoming=incoming;
	m_applicationId=applicationID;
	m_applicationName=applicationName;
	m_cookie= (rand()%(999999))+1;
	m_state = Nothing;
}


MSNInvitation::~MSNInvitation()
{
}

QCString MSNInvitation::unimplemented(long unsigned int cookie)
{
	return QString( "MIME-Version: 1.0\r\n"
					"Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n"
					"\r\n"
					"Invitation-Command: CANCEL\r\n"
					"Cancel-Code: REJECT_NOT_INSTALLED\r\n"
					"Invitation-Cookie: " + QString::number(cookie) + "\r\n"
					"Session-ID: {120019D9-C3F5-4F94-978D-CB33534C3309}\r\n\r\n").utf8();
		//FIXME: i don't know at all what Seession-ID is
}

QString MSNInvitation::invitationHead()
{
	setState(Invited);
	return QString( "MIME-Version: 1.0\r\n"
					"Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n"
					"\r\n"
					"Application-Name: " + m_applicationName + "\r\n"
					"Application-GUID: {" + m_applicationId + "}\r\n"
					"Invitation-Command: INVITE\r\n"
					"Invitation-Cookie: " +QString::number(m_cookie) +"\r\n");
}

QCString MSNInvitation::rejectMessage(const QString & rejectcode)
{
	return QString( "MIME-Version: 1.0\r\n"
					"Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n"
					"\r\n"
					"Invitation-Command: CANCEL\r\n"
					"Invitation-Cookie: " + QString::number(cookie()) + "\r\n"
					"Cancel-Code: "+ rejectcode +"\r\n").utf8();
}

void MSNInvitation::parseInvitation(const QString& msg)
{
	QRegExp rx("Invitation-Command: ([A-Z]*)");
	rx.search(msg);
	QString command=rx.cap(1);

	if(command=="INVITE")
	{
		rx=QRegExp("Invitation-Cookie: ([0-9]*)");
		rx.search(msg);
		m_cookie=rx.cap(1).toUInt();
	}
	else if(command=="CANCEL")
	{
		/*rx=QRegExp("Cancel-Code: ([0-9]*)");
		rx.search(msg);
		QString code=rx.cap(1).toUInt();
		//TODO: parse the code*/
	}
//	else if(command=="ACCEPT")
}

MSNInvitation::State MSNInvitation::state()
{
	return m_state;
}

void MSNInvitation::setState(MSNInvitation::State s)
{
	m_state=s;
}

