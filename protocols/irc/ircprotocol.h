/*
    ircprotocol.h - IRC Protocol

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>

    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

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

#include "kopeteonlinestatus.h"
#include "kopeteprotocol.h"
#include "kopetemimetypehandler.h"

#include "kircentity.h"

#include <dom/dom_node.h>
#include <qdict.h>
#include <qmap.h>

namespace Kopete
{
class Account;
class MetaContact;
}

class AddContactPage;

class EditAccountWidget;
class IRCAccount;

class QStringList;
class QWidget;
class KopeteView;

class IRCProtocolHandler : public Kopete::MimeTypeHandler
{
	public:

		IRCProtocolHandler();

		void handleURL( const KURL &url ) const;
};

static const QString CHAT_VIEW( QString::fromLatin1("kopete_chatwindow") );

/**
 * @author Nick Betcher <nbetcher@kde.org>
 */
class IRCProtocol
	: public Kopete::Protocol
{
	Q_OBJECT

public:
	static IRCProtocol *self();

	IRCProtocol(QObject *parent, const char *name, const QStringList &args);
	~IRCProtocol();

	/**
	 * Kopete::Protocol reimplementation
	 */
	virtual AddContactPage *createAddContactWidget(QWidget *parent, Kopete::Account *account);

	/**
	 * Deserialize contact data
	 */
	virtual Kopete::Contact *deserializeContact( Kopete::MetaContact *metaContact,
		const QMap<QString, QString> &serializedData, const QMap<QString, QString> &addressBookData );

	virtual KopeteEditAccountWidget* createEditAccountWidget(Kopete::Account *account, QWidget *parent);

	virtual Kopete::Account* createNewAccount(const QString &accountId);

	virtual QPtrList<KAction> *customChatWindowPopupActions(const Kopete::Message &, DOM::Node &);

	Kopete::OnlineStatus onlineStatusFor(const KIRC::EntityStatus &status);

	bool commandInProgress(){ return m_commandInProgress; }
	void setCommandInProgress( bool ip ) { m_commandInProgress = ip; }

public slots:
	void editNetworks(const QString &networkName);

private slots:
	void slotMessageFilter(Kopete::Message &msg);

	void slotRawCommand(const QString &args, Kopete::ChatSession *manager );
	void slotQuoteCommand(const QString &args, Kopete::ChatSession *manager );
	void slotCtcpCommand(const QString &args, Kopete::ChatSession *manager );
	void slotPingCommand(const QString &args, Kopete::ChatSession *manager );

	void slotMotdCommand(const QString &args, Kopete::ChatSession *manager);
	void slotListCommand(const QString &args, Kopete::ChatSession *manager);
	void slotTopicCommand(const QString &args, Kopete::ChatSession *manager);
	void slotJoinCommand(const QString &args, Kopete::ChatSession *manager);
	void slotNickCommand(const QString &args, Kopete::ChatSession *manager);
	void slotWhoisCommand(const QString &args, Kopete::ChatSession *manager);
	void slotWhoWasCommand(const QString &args, Kopete::ChatSession *manager);
	void slotWhoCommand(const QString &args, Kopete::ChatSession *manager);
	void slotMeCommand(const QString &args, Kopete::ChatSession *manager);
	void slotAllMeCommand(const QString &args, Kopete::ChatSession *manager);
	void slotModeCommand(const QString &args, Kopete::ChatSession *manager);
	void slotQueryCommand(const QString &args, Kopete::ChatSession *manager);

	void slotKickCommand(const QString &args, Kopete::ChatSession *manager);
	void slotBanCommand(const QString &args, Kopete::ChatSession *manager);
	void slotOpCommand(const QString &args, Kopete::ChatSession *manager);
	void slotDeopCommand(const QString &args, Kopete::ChatSession *manager);
	void slotVoiceCommand(const QString &args, Kopete::ChatSession *manager);
	void slotDevoiceCommand(const QString &args, Kopete::ChatSession *manager);
	void slotQuitCommand(const QString &args, Kopete::ChatSession *manager);
	void slotPartCommand(const QString &args, Kopete::ChatSession *manager);
	void slotInviteCommand(const QString &args, Kopete::ChatSession *manager);

	void slotViewCreated(KopeteView *);

private:
	void initOnlineStatus();
	Kopete::OnlineStatus onlineStatusFor(const KIRC::EntityStatus &status, unsigned categories);
	void simpleModeChange(const QString &, Kopete::ChatSession *, const QString &mode);

	static IRCProtocol *s_protocol;

	QMap<KIRC::EntityStatus, Kopete::OnlineStatus> m_statusMap;
//	const Kopete::OnlineStatus m_connecting;
	const Kopete::OnlineStatus m_StatusUnknown;

	DOM::Node activeNode;
	IRCAccount *activeAccount;

	bool m_commandInProgress;

	IRCProtocolHandler *m_protocolHandler;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

