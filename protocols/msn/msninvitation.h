/*
    msninvitation.cpp

    Copyright (c) 2003 by Olivier Goffart        <ogoffart@tiscalinet.be>

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
#ifndef MSNINVITATION_H
#define MSNINVITATION_H

#include <qstring.h>

/**
 * @author Olivier Goffart
*/
class MSNInvitation
{
public:

	MSNInvitation(bool incomming,const QString &applicationID , const QString &applicationName);
	virtual ~MSNInvitation();
	void setCookie( long unsigned int c ) { m_cookie = c; }
	long unsigned int cookie() { return m_cookie; }
	bool incoming() { return m_incoming; }

	static QCString unimplemented(long unsigned int cookie);
	QString invitationHead();

    virtual void parseInvitation(const QString& invitation);


protected:
	bool m_incoming;
	long unsigned int m_cookie;
	QString m_applicationId;
	QString m_applicationName;


};

#endif
