/***************************************************************************
                          irccontact.h  -  description
                             -------------------
    begin                : Wed Mar 6 2002
    copyright            : (C) 2002 by nbetcher
    email                : nbetcher@usinternet.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef IRCCONTACT_H
#define IRCCONTACT_H



#include "kopetecontact.h"

#include <qstring.h>



class IRCChatView;
class IRCQueryView;
class IRCServerContact;
class KIRC;

class QStringList;
class QVBox;



class IRCContact : public KopeteContact
{
	Q_OBJECT
public:
	IRCContact(const QString &server, const QString &target, unsigned int port, bool joinOnConnect, IRCServerContact *contact, KopeteMetaContact *parent, QString &protocolID);
	IRCContact(const QString &server, const QString &target, unsigned int port, bool joinOnConnect, IRCServerContact *contact, const QStringList pengingMessages, KopeteMetaContact *parent, QString &protocolID);
	IRCContact(const QString &groupName, const QString &server, const QString &target, unsigned int port, bool joinOnConnect, IRCServerContact *contact, KopeteMetaContact *parent, QString &protocolID);
	~IRCContact();
	// KopeteContact virtual functions
	virtual ContactStatus status() const;
	virtual QString statusIcon() const;
	virtual bool isReachable() { return true; }
	virtual KActionCollection *customContextMenuActions();
	
	bool waitingPart() { return m_waitingPart ;};
	bool requestedQuit() { return m_requestedQuit; };

	IRCServerContact *serverContact() { return m_serverContact; };
	KIRC *engine() { return m_engine; };

	QVBox *tabPage() { return mTabPage; };
	IRCChatView *getChatView() { return chatView; };

	QString serverName() { return m_serverName; };
	QString targetName() { return m_targetName; } ;
	QString groupName() { return m_groupName; };

	virtual QString id() const;

private:

	bool init();

	/**
	 * Returns true, if targetName() begins with '#', '&', '!' or '+'.
	 */
	bool isChannel() const;

	QString m_serverName;
	QString m_targetName;
	QString m_groupName;

	bool m_waitingPart;
	bool m_requestedQuit;


	KIRC *m_engine;
	IRCServerContact *m_serverContact;
	unsigned int m_port;

	QString m_username;
	QString m_nickname;

	bool mJoinOnConnect;
	IRCChatView *chatView;
	QVBox *mTabPage;
	IRCQueryView *queryView;
	bool added;
	bool contactOnList;

	KActionCollection* m_pActionCollection;

private slots:
//	void slotHop();
	void slotPartedChannel(const QString &, const QString &, const QString &);
	void slotUserKicked(const QString &, const QString &, const QString &, const QString &);
	void slotOpen();
	void slotOpenConnect();
	void incomingPrivMessage(const QString &, const QString &, const QString &);
	void incomingPrivAction(const QString &, const QString &, const QString &);

public slots:

	void slotPart();
	void joinNow();
	void unloading();

	virtual void execute();
	virtual void slotViewHistory() {}
	virtual void slotDeleteContact();
	virtual void slotUserInfo() {}
};

#endif
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

