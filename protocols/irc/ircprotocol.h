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

#include <qpixmap.h>

class KopeteMetaContact;
class AddContactPage;
class IRCServerManager;
class KIRC;

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
	void init();
	void deserialize( KopeteMetaContact *metaContact, const QStringList &strList );

	/** KopeteProtocol reimplementation */
	virtual QString protocolIcon() const;
	virtual AddContactPage *createAddContactWidget(QWidget *parent);
	virtual void Connect();
	virtual void Disconnect();
	virtual bool isConnected() const;
	virtual void setAway(void);
	virtual void setAvailable(void);
	virtual bool isAway(void) const;
	virtual KActionMenu* protocolActions();

	//following implementation is incorrect
	KopeteContact* myself() const { return 0L; }

	void addContact(const QString &, const QString &, bool, bool,KopeteMetaContact *m=0l);

	IRCServerManager *serverManager() { return m_serverManager; }
	KIRC *engine() { return m_engine; };

public slots:
	void serialize(KopeteMetaContact * metaContact);

private slots:
	void slotNewConsole();

private:
	/** import contact-list from kopete 0.4.x */
	void importOldContactList();
	KActionMenu *m_actionMenu;
	IRCServerManager *m_serverManager;
	KIRC *m_engine;
	bool m_isConnected;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

