/*
    jabbermessagemanager.h - Jabber Message Manager

    Copyright (c) 2004 by Till Gerken            <till@tantalo.net>

    Kopete    (c) 2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef JABBERMESSAGEMANAGER_H
#define JABBERMESSAGEMANAGER_H

#include "kopetemessagemanager.h"

class JabberProtocol;
class JabberAccount;
class JabberBaseContact;
namespace Kopete { class Message; }
class QString;

/**
 * @author Till Gerken
 */
class JabberMessageManager : public Kopete::MessageManager
{
	Q_OBJECT

public:
	JabberMessageManager ( JabberProtocol *protocol, const JabberBaseContact *user,
						   KopeteContactPtrList others, const QString &resource = "",
						   const char *name = 0 );

	/**
	 * @brief Get the local user in the session
	 * @return the local user in the session, same as account()->myself()
	 */
	const JabberBaseContact *user () const;

	/**
	 * @brief get the account
	 * @return the account
	 */
	JabberAccount *account() const ;

	/**
	 * @brief Return the resource this manager is currently associated with.
	 * @return currently associated resource
	 */
	const QString &resource () const;

	/**
	 * Re-generate the display name
	 */
	void updateDisplayName ();

public slots:
	/**
	 * Show a message to the chatwindow, or append it to the queue.
	 * This is the function protocols HAVE TO call for both incoming and outgoing messages
	 * if the message must be showed in the chatwindow
	 *
	 * This is an overloaded version of the original implementation which
	 * also accepts a resource the message originates from. The message manager
	 * will set its own resource to the resource the message was received from.
	 * See @ref JabberBaseContact::manager() about how to deal with instantiating
	 * new message managers for messages not originating from the same resource
	 * a manager already exists for.
	 */
	void appendMessage ( Kopete::Message &msg, const QString &fromResource );

private slots:
	void slotSendTypingNotification ( bool typing );
	void slotMessageSent ( Kopete::Message &message, Kopete::MessageManager *kmm );

private:
	QString mResource;

};

#endif

// vim: set noet ts=4 sts=4 tw=4:

