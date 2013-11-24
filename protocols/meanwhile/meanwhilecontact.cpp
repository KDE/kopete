/*
    meanwhilecontact.cpp - a meanwhile contact

    Copyright (c) 2003-2004 by Sivaram Gottimukkala  <suppandi@gmail.com>

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

#include "meanwhilecontact.h"

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <QList>

#include "kopeteaccount.h"
#include "kopetechatsessionmanager.h"
#include "kopetemetacontact.h"

#include "meanwhileprotocol.h"
#include "meanwhilesession.h"
#include "meanwhileaccount.h"
#include "meanwhileplugin.h"

MeanwhileContact::MeanwhileContact(QString userId, QString nickname,
		MeanwhileAccount *account, Kopete::MetaContact *parent)
		: Kopete::Contact(account, userId, parent)
{
	setNickName(nickname);
	m_msgManager = 0L;
	m_meanwhileId = userId;
	setOnlineStatus(static_cast<MeanwhileProtocol *>(account->protocol())
			->statusOffline);
}

MeanwhileContact::~MeanwhileContact()
{
}

bool MeanwhileContact::isReachable()
{
    return isOnline();
}

void MeanwhileContact::serialize(QMap<QString, QString> &serializedData,
		QMap<QString, QString> &addressBookData)
{
	Kopete::Contact::serialize(serializedData, addressBookData);
}

void MeanwhileContact::showContactSettings()
{
}

void MeanwhileContact::slotUserInfo()
{
	MeanwhileAccount *theAccount = static_cast<MeanwhileAccount *>( account());
	theAccount->infoPlugin->showUserInfo(m_meanwhileId);
}

Kopete::ChatSession* MeanwhileContact::manager(CanCreateFlags canCreate)
{
	if (m_msgManager != 0L || canCreate == Kopete::Contact::CannotCreate)
		return m_msgManager;

	QList<Kopete::Contact*> contacts;
	contacts.append(this);
	m_msgManager = Kopete::ChatSessionManager::self()->
		create(account()->myself(), contacts, protocol());

	connect(m_msgManager,
			SIGNAL(messageSent(Kopete::Message&,Kopete::ChatSession*)),
			this, SLOT(sendMessage(Kopete::Message&)));

	connect(m_msgManager, SIGNAL(myselfTyping(bool)),
			this, SLOT(slotSendTyping(bool)));

	connect(m_msgManager, SIGNAL(destroyed()),
			this, SLOT(slotChatSessionDestroyed()));

	return m_msgManager;
}

QString MeanwhileContact::meanwhileId() const
{
	return m_meanwhileId;
}

void MeanwhileContact::sendMessage(Kopete::Message &message)
{
	static_cast<MeanwhileAccount *>(account())->session()->sendMessage(message);
}

void MeanwhileContact::slotSendTyping(bool isTyping)
{
	static_cast<MeanwhileAccount *>(account())->session()->
		sendTyping(this, isTyping);
}

void MeanwhileContact::receivedMessage(const QString &message)
{
	Kopete::Message kmessage(this, account()->myself());
	kmessage.setPlainBody(message);
	kmessage.setDirection(Kopete::Message::Inbound);

	manager(Kopete::Contact::CanCreate)->appendMessage(kmessage);
}

void MeanwhileContact::sync(unsigned int changed)
{
	if (changed)
		static_cast<MeanwhileAccount *>(account())->syncContactsToServer();
}

void MeanwhileContact::slotChatSessionDestroyed()
{
	m_msgManager->deref();
	m_msgManager = 0L;
}

#include "meanwhilecontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

