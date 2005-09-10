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
#ifndef MSNINVITATION_H
#define MSNINVITATION_H

#include <qstring.h>

#include "kopete_export.h"

class QObject;

/**
 * @author Olivier Goffart
 *
 * The invitation is the base class which handle an MSN invitation.
 * The implemented class must to herits from QObject too.
 * You can accept the invitation by catching @ref MSNProtocol::invitation() signals
 * or create one and insert it to a kmm with @ref MSNChatSession::initInvitation()
 * you can add action with @ref Kopete::Plugin::customChatActions()
 */
class KOPETE_EXPORT MSNInvitation
{
public:
	/**
	 * Constructor
	 * @param incoming say if it is an incoming invitation
	 * @param applicationID is the exadecimal id of the invitation
	 * @param applicationName is a i18n'ed string of the name of the application
	 */
	MSNInvitation(bool incoming,const QString &applicationID , const QString &applicationName);
	virtual ~MSNInvitation();

	/**
	 * @internal
	 * it is a reject invitation because the invitation is not implemented
	 */
	static QCString unimplemented(long unsigned int cookie);

	/**
	 * you can set manualy the cookie. note that a cookie is automatically generated when a new
	 * invitation is created, or in @ref parseInvitation
	 */
	void setCookie( long unsigned int c ) { m_cookie = c; }
	/**
	 * @return the cookie
	 */
	long unsigned int cookie() { return m_cookie; }

	/**
	 * @return true if it is an incommijng invitation
	 */
	bool incoming() { return m_incoming; }


	/**
	 * reimplement this. this is the invitation string used in @ref MSNChatSession::initInvitation()
	 * the default implementation return the common begin.
	 * You can also set the state to Invited (the default implementation do that)
	 */
	virtual QString invitationHead();

	/**
	 * This is the reject invitation string
	 * @param rejectcode is the code, it can be "REJECT" or "TIMEOUT"
	 */
	QCString rejectMessage(const QString & rejectcode = "REJECT");

	/**
	 * reimplement this method. it is called when an invitation message with the invitation's cookie is received
	 * the default implementation parse the cookie, or the reject message
	 */
	virtual void parseInvitation(const QString& invitation);

	/**
	 * return the qobject (this)
	 */
	virtual QObject* object()=0;
//signals:
	/**
	 * reimplement this as a signal, and emit it when the invitation has to be destroyed.
	 * don't delete the invitation yourself
	 */
	virtual void done(MSNInvitation*)=0;

	/**
	 * This indiquate the state. it is going to be completed later
	 * - Nothing means than nothing has been done in the invitaiton (nothing has been sent/received)
	 * - Invited means than the invitaiton has been sent
	 */
	enum State { Nothing=0 , Invited=1 };
	/**
	 * retrun the current state
	 */
	State state();
	/**
	 * set the current State
	 */
	void setState(State);



protected:
	bool m_incoming;
	long unsigned int m_cookie;
	QString m_applicationId;
	QString m_applicationName;
	State m_state;


};

#endif
