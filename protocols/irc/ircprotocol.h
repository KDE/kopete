/*
    ircprotocol.h - IRC Protocol

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>

    Kopete    (c) 2002      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef IRCPROTOCOL_H
#define IRCPROTOCOL_H

#include "kopeteprotocol.h"

class KopeteMetaContact;
class AddContactPage;
class KIRC;

class KAction;
class KActionMenu;
class EditIdentityWidget;
class KopeteIdentity;
class IRCIdentity;

class QStringList;
class QWidget;

/**
 * @author Nick Betcher <nbetcher@kde.org>
 */

class IRCProtocol : public KopeteProtocol
{
	Q_OBJECT

public:
	IRCProtocol( QObject *parent, const char *name, const QStringList &args );
	~IRCProtocol();
	/* Plugin reimplementation */
	virtual void init();

	/** KopeteProtocol reimplementation */
	virtual AddContactPage *createAddContactWidget(QWidget *parent);
	virtual KActionMenu* protocolActions();

	/**
	 * Deserialize contact data
	 */
	virtual void deserializeContact( KopeteMetaContact *metaContact,
		const QMap<QString, QString> &serializedData, const QMap<QString, QString> &addressBookData );
	virtual const QString protocolIcon();

	virtual EditIdentityWidget* createEditIdentityWidget(KopeteIdentity *identity, QWidget *parent);

	virtual KopeteIdentity* createNewIdentity(const QString &identityId);

public slots:
	/** OBSOLETE **/
	virtual void connect();
	virtual void disconnect();

private:
	KActionMenu *m_actionMenu;

	/** FIXME: Do something with this when Identity support is added!!!!!!!! */
	IRCIdentity *identity;
	KAction *actionDisconnect;
	KAction *actionConnect;

	QMap<QString,IRCIdentity*> mIdentityMap;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

