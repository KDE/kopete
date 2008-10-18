/*
    ircprotocol.h - IRC Protocol

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>
    Copyright (c) 2003-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

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

#include "kircentity.h"
#include "kircglobal.h"

#include "kopetemessage.h"
#include "kopeteonlinestatus.h"
#include "kopeteprotocol.h"
#include "kopetemimetypehandler.h"

//#include <dom/dom_node.h>

#include <QMap>

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

		void handleURL( const KUrl &url ) const;
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

	IRCProtocol(QObject *parent, const QVariantList &args);
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

//	virtual QList<KAction *> *customChatWindowPopupActions(const Kopete::Message &, DOM::Node &);

	Kopete::OnlineStatus onlineStatusFor(KIrc::EntityPtr entity);

	bool commandInProgress(){ return m_commandInProgress; }
	void setCommandInProgress( bool ip ) { m_commandInProgress = ip; }

public slots:
	void editNetworks(const QString &networkName);

private slots:
	void slotMessageFilter(Kopete::Message &msg);

	void slotAllCommand(const QString &args, Kopete::ChatSession *manager );
	void slotCtcpCommand(const QString &args, Kopete::ChatSession *manager );
	void slotQuoteCommand(const QString &args, Kopete::ChatSession *manager );
	void slotRawCommand(const QString &args, Kopete::ChatSession *manager );

	void slotViewCreated(KopeteView *);

private:
	void initOnlineStatus();
	void simpleModeChange(const QString &, Kopete::ChatSession *, const QString &mode);

	//QMap<KIrc::EntityStatus, Kopete::OnlineStatus> m_statusMap;
//	const Kopete::OnlineStatus m_connecting;
	const Kopete::OnlineStatus m_StatusUnknown;

//	DOM::Node activeNode;
	IRCAccount *activeAccount;

	bool m_commandInProgress;

	IRCProtocolHandler *m_protocolHandler;
};

#endif

