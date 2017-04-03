/*
    yahooconferencemessagemanager.h - Yahoo Conference Message Manager

    Copyright (c) 2003 by Duncan Mac-Vicar <duncan@kde.org>
    Copyright (c) 2005 by André Duffeck        <duffeck@kde.org>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "yahooconferencemessagemanager.h"

#include "yahoo_protocol_debug.h"
#include <QLineEdit>
#include <KLocalizedString>
#include <kmessagebox.h>
#include <QMenu>
#include <kconfig.h>

#include <kopetecontactaction.h>
#include <kopetecontactlist.h>
#include <kopetecontact.h>
#include <kopetechatsessionmanager.h>
#include <kopeteuiglobal.h>
#include <QIcon>

#include "yahoocontact.h"
#include "yahooaccount.h"
#include "yahooinvitelistimpl.h"
#include <kactioncollection.h>

YahooConferenceChatSession::YahooConferenceChatSession( const QString & yahooRoom, Kopete::Protocol *protocol, const Kopete::Contact *user,
	Kopete::ContactPtrList others )
: Kopete::ChatSession( user, others, protocol )
{

    setComponentName(QStringLiteral("yahoo_protocol"), i18n("Kopete"));
	Kopete::ChatSessionManager::self()->registerChatSession( this );

	connect ( this, SIGNAL(messageSent(Kopete::Message&,Kopete::ChatSession*)),
			  SLOT(slotMessageSent(Kopete::Message&,Kopete::ChatSession*)) );

	m_yahooRoom = yahooRoom;

    m_actionInvite = new QAction( QIcon::fromTheme(QStringLiteral("x-office-contact")), i18n( "&Invite others" ), this ); // icon should probably be "contact-invite", but that doesn't exist... please request an icon on http://techbase.kde.org/index.php?title=Projects/Oxygen/Missing_Icons
        actionCollection()->addAction( QStringLiteral("yahooInvite"), m_actionInvite );
	connect ( m_actionInvite, SIGNAL(triggered(bool)), this, SLOT(slotInviteOthers()) );

	setXMLFile(QStringLiteral("yahooconferenceui.rc"));
}

YahooConferenceChatSession::~YahooConferenceChatSession()
{
	emit leavingConference( this );
}

YahooAccount *YahooConferenceChatSession::account()
{
	return static_cast< YahooAccount *>( Kopete::ChatSession::account() );
}

const QString &YahooConferenceChatSession::room()
{
	return m_yahooRoom;
}

void YahooConferenceChatSession::joined( YahooContact *c )
{
	addContact( c );
}

void YahooConferenceChatSession::left( YahooContact *c )
{
	removeContact( c );
}

void YahooConferenceChatSession::slotMessageSent( Kopete::Message & message, Kopete::ChatSession * )
{
    qCDebug (YAHOO_PROTOCOL_LOG) ;

	YahooAccount *acc = dynamic_cast< YahooAccount *>( account() );
	if( acc )
		acc->sendConfMessage( this, message );
	appendMessage( message );
	messageSucceeded();
}

void YahooConferenceChatSession::slotInviteOthers()
{
	QStringList buddies;

	QHash<QString, Kopete::Contact*>::ConstIterator it, itEnd = account()->contacts().constEnd();
	for( it = account()->contacts().constBegin(); it != itEnd; ++it )
	{
		if( !members().contains( it.value() ) )
			buddies.push_back( it.value()->contactId() );
	}

	YahooInviteListImpl *dlg = new YahooInviteListImpl( Kopete::UI::Global::mainWidget() );
	QObject::connect( dlg, SIGNAL(readyToInvite(QString,QStringList,QStringList,QString)),
				account(), SLOT(slotAddInviteConference(QString,QStringList,QStringList,QString)) );
	dlg->setRoom( m_yahooRoom );
	dlg->fillFriendList( buddies );
	for( QList<Kopete::Contact*>::ConstIterator it = members().constBegin(); it != members().constEnd(); ++it )
		dlg->addParticipant( (*it)->contactId() );
	dlg->show();
}

