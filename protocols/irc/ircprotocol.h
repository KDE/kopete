/***************************************************************************
                          msnprotocol.h  -  description
                             -------------------
    begin                : Wed Jan 2 2002
    copyright            : (C) 2002 by duncan
    email                : duncan@tarro
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef IRCPROTOCOL_H
#define IRCPROTOCOL_H

#include "kopeteprotocol.h"
#include "ircidentity.h"
#include "ircusercontact.h"

#include <qpixmap.h>

class KopeteMetaContact;
class AddContactPage;
class IRCServerManager;
class KIRC;

class KAction;
class KActionMenu;
class KSimpleConfig;

class QStringList;
class QWidget;

/**
 * @author duncan
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
	virtual bool isConnected() const;
	virtual void setAway(void);
	virtual void setAvailable(void);
	virtual bool isAway(void) const;
	virtual KActionMenu* protocolActions();

	// FIXME WHEN IDENTITY SUPPORT IS ADDED:
	virtual KopeteContact *myself() const { return static_cast<KopeteContact*>( identity->mySelf() ); }

	void addContact(const QString &, const QString &, bool isChannel, KopeteMetaContact *m=0l);

	/**
	 * Deserialize contact data
	 */
	virtual void deserializeContact( KopeteMetaContact *metaContact,
		const QMap<QString, QString> &serializedData, const QMap<QString, QString> &addressBookData );
	virtual const QString protocolIcon();

public slots:
	virtual void connect();
	virtual void disconnect();
private slots:
	void slotConnectedToServer();
	void slotConnectionClosed();
private:
	KActionMenu *m_actionMenu;
	bool m_isConnected;
	/** FIXME: Do something with this when Identity support is added!!!!!!!! */
	IRCIdentity *identity;
	KAction *actionDisconnect;
	KAction *actionConnect;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

