/*
    netmeetinginvitation.cpp

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
#ifndef MSNVOICEINVITATION_H
#define MSNVOICEINVITATION_H

#include <qobject.h>
#include "msninvitation.h"

class MSNContact;

/**
 *@author Olivier Goffart
 */
class NetMeetingInvitation : public QObject , public MSNInvitation
{
Q_OBJECT
public:
	NetMeetingInvitation(bool incoming ,MSNContact*, QObject *parent = 0);
	~NetMeetingInvitation();

	static QString applicationID() { return "44BBA842-CC51-11CF-AAFA-00AA00B6015C"; }
	QString invitationHead();

	virtual void parseInvitation(const QString& invitation);

	virtual QObject* object() { return this; }

signals:
	void done( MSNInvitation * );

private slots:
	void slotTimeout();

private:
	MSNContact *m_contact;
	bool oki;
	void startMeeting(const QString & ip_address);

};


#endif
