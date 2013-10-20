/*
    gwmessagemanager.cpp - Kopete GroupWise Protocol

    Copyright (c) 2006,2007 Novell, Inc	 	 	 http://www.opensuse.org
    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com

    Based on Testbed
    Copyright (c) 2003-2007 by Will Stephenson		 <wstephenson@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "gwmessagemanager.h"
#include <qlabel.h>
#include <qvalidator.h>
#include <QList>
#include <QPainter>

#include <kactioncollection.h>
#include <kactionmenu.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kxmlguiwindow.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kshortcut.h>

#include <kopetecontact.h>
#include <kopetecontactaction.h>
#include <kopetemetacontact.h>
#include <kopetechatsessionmanager.h>
#include <kopeteprotocol.h>
#include <kopeteuiglobal.h>
#include <kopeteview.h>

#include "client.h"
#include "gwaccount.h"
#include "gwcontact.h"
#include "gwerror.h"
#include "gwprotocol.h"
#include "gwsearch.h"

GroupWiseChatSession::GroupWiseChatSession(const Kopete::Contact* user, Kopete::ContactPtrList others, Kopete::Protocol* protocol, const GroupWise::ConferenceGuid & guid, int id ): Kopete::ChatSession( user, others, protocol ), m_guid( guid ), m_flags( 0 ), m_searchDlg( 0 ), m_memberCount( others.count() )
{
	Q_UNUSED( id );
	static int s_id=0;
	m_mmId=++s_id;

	kDebug () << "New message manager for " << user->contactId();

	// Needed because this is (indirectly) a KXMLGuiClient, so it can find the gui description .rc file
	setComponentData( protocol->componentData() );

	// make sure Kopete knows about this instance
	Kopete::ChatSessionManager::self()->registerChatSession ( this );

	connect ( this, SIGNAL(messageSent(Kopete::Message&,Kopete::ChatSession*)),
			  SLOT(slotMessageSent(Kopete::Message&,Kopete::ChatSession*)) );
	connect( this, SIGNAL(myselfTyping(bool)), SLOT(slotSendTypingNotification(bool)) );
	connect( account(), SIGNAL(contactTyping(ConferenceEvent)),
						SLOT(slotGotTypingNotification(ConferenceEvent)) );
	connect( account(), SIGNAL(contactNotTyping(ConferenceEvent)),
						SLOT(slotGotNotTypingNotification(ConferenceEvent)) );

	// Set up the Invite menu
	m_actionInvite = new KActionMenu( i18n( "&Invite" ), this );
	actionCollection()->addAction( "gwInvite", m_actionInvite );
	connect( m_actionInvite->menu(), SIGNAL(aboutToShow()), this, SLOT(slotActionInviteAboutToShow()) ) ;

	m_secure = new KAction( KIcon( "security-high" ), i18n( "Security Status" ), 0 ); // "gwSecureChat"
	QObject::connect( m_secure, SIGNAL(triggered(bool)), SLOT(slotShowSecurity()) );
	m_secure->setToolTip( i18n( "Conversation is secure" ) );

	m_logging = new KAction( KIcon( "utilities-log-viewer" ), i18n( "Archiving Status" ), 0 ); // "gwLoggingChat"
	QObject::connect( m_secure, SIGNAL(triggered(bool)),  SLOT(slotShowArchiving()) );
	updateArchiving();

	setXMLFile("gwchatui.rc");
	setMayInvite( true );
}

GroupWiseChatSession::~GroupWiseChatSession()
{
	qDeleteAll(m_inviteActions);
	emit leavingConference( this );
	foreach (Kopete::Contact * contact, m_invitees )
		delete contact;
}

int GroupWiseChatSession::mmId() const
{
  return m_mmId;
}

void GroupWiseChatSession::setGuid( const GroupWise::ConferenceGuid & guid )
{
	if ( m_guid.isEmpty() )
	{
		kDebug() << "setting GUID to: " << guid;
		m_guid = guid;
	}
	else
		kDebug() << "attempted to change the conference's GUID when already set!";
}

bool GroupWiseChatSession::closed()
{
	return m_flags & GroupWise::Closed;
}

bool GroupWiseChatSession::logging()
{
	return m_flags & GroupWise::Logging;
}

bool GroupWiseChatSession::secure()
{
	return m_flags & GroupWise::Secure;
}

void GroupWiseChatSession::setClosed()
{
	kDebug() << " Conference " << m_guid << " is now Closed ";
	m_guid.clear();
	m_flags = m_flags | GroupWise::Closed;
}

void GroupWiseChatSession::setLogging( bool logging )
{
	if ( logging )
		m_flags = m_flags | GroupWise::Logging;
	else
		m_flags = m_flags & !GroupWise::Logging;
}

void GroupWiseChatSession::setSecure( bool secure )
{
	if ( secure )
		m_flags = m_flags | GroupWise::Secure;
	else
		m_flags = m_flags & !GroupWise::Secure;
}

GroupWiseAccount *  GroupWiseChatSession::account()
{
	return static_cast<GroupWiseAccount *>( Kopete::ChatSession::account() );
}

void GroupWiseChatSession::createConference()
{
	if ( m_guid.isEmpty() )
	{
		kDebug () ;
		// form a list of invitees
		QStringList invitees;
		foreach ( Kopete::Contact * contact, members() )
		{
			invitees.append( static_cast< GroupWiseContact * >( contact )->dn() );
		}
		// this is where we will set the GUID and send any pending messages
		connect( account(), SIGNAL(conferenceCreated(int,GroupWise::ConferenceGuid)), SLOT(receiveGuid(int,GroupWise::ConferenceGuid)) );
		connect( account(), SIGNAL(conferenceCreationFailed(int,int)), SLOT(slotCreationFailed(int,int)) );

		// create the conference
		account()->createConference( mmId(), invitees );
	}
	else
		kDebug () << " tried to create conference on the server when it was already instantiated";
}

void GroupWiseChatSession::receiveGuid( const int newMmId, const GroupWise::ConferenceGuid & guid )
{
	if ( newMmId == mmId() )
	{
		kDebug () << " got GUID from server";
		m_memberCount = members().count();
		setGuid( guid );
		// re-add all the members.  This is because when the last member leaves the conference,
		// they are removed from the chat member list GUI.  By re-adding them here, we guarantee they appear
		// in the UI again, at the price of a debug message when starting up a new chatwindow

		foreach( Kopete::Contact * contact, members() )
			addContact( contact, true );

		// notify the contact(s) using this message manager that it's been instantiated on the server
		emit conferenceCreated();
		// TODO: send invitations if we're not inviting in the conf create...
		dequeueMessagesAndInvites();
	}
}

void GroupWiseChatSession::slotCreationFailed( const int failedId, const int statusCode )
{
	if ( failedId == mmId() )
	{
		kDebug () << " could not start a chat, no GUID.\n";
		//emit creationFailed();
		Kopete::Message failureNotify( myself(), members());
		failureNotify.setPlainBody( i18n("An error occurred when trying to start a chat: %1", statusCode ) );
		appendMessage( failureNotify );
		setClosed();
	}
}

void GroupWiseChatSession::slotSendTypingNotification( bool typing )
{
	// only send a notification if we've got a conference going and we are not Appear Offline
	if ( !m_guid.isEmpty() && m_memberCount &&
		  ( account()->myself()->onlineStatus() != GroupWiseProtocol::protocol()->groupwiseAppearOffline ) )
				account()->client()->sendTyping( guid(), typing );
}

void GroupWiseChatSession::slotMessageSent( Kopete::Message & message, Kopete::ChatSession * )
{
	kDebug () ;
	if( account()->isConnected() )
	{
		/*if ( closed() )
		{
			Kopete::Message failureNotify = Kopete::Message( myself(), members(), i18n("Your message could not be sent. This conversation has been closed by the server, because all the other participants left or declined invitations. "), Kopete::Message::Internal, Kopete::Message::PlainText);
			appendMessage( failureNotify );
			messageSucceeded();
		}
		else*/ if ( account()->myself()->onlineStatus() == ( static_cast<GroupWiseProtocol *>( protocol() ) )->groupwiseAppearOffline )
		{
			Kopete::Message failureNotify( myself(), members() );
			failureNotify.setPlainBody( i18n("Your message could not be sent. You cannot send messages while your status is Appear Offline. ") );
			appendMessage( failureNotify );
			messageSucceeded();
		}
		else
		{
			// if the conference has not been instantiated yet, or if all the members have left
			if ( m_guid.isEmpty() || m_memberCount == 0 )
			{
				// if there are still invitees, the conference is instantiated, and there are only
				if ( m_invitees.count() )
				{
					// the message won't go anywhere, as there's none there except invitees, but we warn the user
					// when the last participant leaves.
					messageSucceeded();
				}
				else
				{
					kDebug () << "waiting for server to create a conference, queuing message";
					// the conference hasn't been instantiated on the server yet, so queue the message
					m_guid = ConferenceGuid();
					createConference();
					m_pendingOutgoingMessages.append( message );
				}
			}
			else
			{
				kDebug () << "sending message";
				account()->sendMessage( guid(), message );
				// we could wait until the server acks our send,
				// but we'd need a UID for outgoing messages and a list to track them
				appendMessage( message );
				messageSucceeded();
			}
		}
	}
}

void GroupWiseChatSession::slotGotTypingNotification( const ConferenceEvent& event )
{
	if ( event.guid == guid() )
		receivedTypingMsg( static_cast<GroupWiseProtocol *>( protocol() )->dnToDotted( event.user ), true );
}

void GroupWiseChatSession::slotGotNotTypingNotification( const ConferenceEvent& event )
{
	if ( event.guid == guid() )
		receivedTypingMsg( static_cast<GroupWiseProtocol *>( protocol() )->dnToDotted( event.user ), false );
}

void GroupWiseChatSession::dequeueMessagesAndInvites()
{
	kDebug () ;
	for ( QList< Kopete::Message >::Iterator it = m_pendingOutgoingMessages.begin();
		  it != m_pendingOutgoingMessages.end();
		  ++it )
	{
		slotMessageSent( *it, this );
	}
	m_pendingOutgoingMessages.clear();

	foreach( Kopete::Contact * contact, m_pendingInvites )
		slotInviteContact( contact );
	m_pendingInvites.clear();
}

void GroupWiseChatSession::slotActionInviteAboutToShow()
{
	// We can't simply insert  KAction in this menu bebause we don't know when to delete them.
	//  items inserted with insert items are automatically deleted when we call clear

	qDeleteAll(m_inviteActions);
	m_inviteActions.clear();

	m_actionInvite->menu()->clear();


	foreach( Kopete::Contact * contact, account()->contacts() )
	{
		if( !members().contains( contact ) && contact->isOnline() )
		{
			KAction *a = new Kopete::UI::ContactAction( contact,
			                                            actionCollection() );
			m_actionInvite->addAction( a );
			QObject::connect( a, SIGNAL(triggered(Kopete::Contact*,bool)),
					this, SLOT(slotInviteContact(Kopete::Contact*)) );
			m_inviteActions.append( a ) ;
		}
	}
	// Invite someone off-list
	KAction *b = new KAction( i18n("&Other..."), this );
	actionCollection()->addAction( "actionOther", b );
	QObject::connect( b, SIGNAL(triggered(bool)),
	                  this, SLOT(slotInviteOtherContact()) );
	m_actionInvite->addAction( b );
	m_inviteActions.append( b ) ;
}

void GroupWiseChatSession::slotInviteContact( Kopete::Contact * contact )
{
	if ( m_guid.isEmpty() )
	{
		m_pendingInvites.append( contact );
		createConference();
	}
	else
	{
		QWidget * w = view(false) ? dynamic_cast<KMainWindow*>( view(false)->mainWidget()->topLevelWidget() ) : 0L;

		bool ok;
		QRegExp rx( ".*" );
		QRegExpValidator validator( rx, this );
		QString inviteMessage = KInputDialog::getText( i18n( "Enter Invitation Message" ),
		    i18n( "Enter the reason for the invitation, or leave blank for no reason:" ), QString(), &ok, w ? w : Kopete::UI::Global::mainWidget(), &validator );
		if ( ok )
		{
			GroupWiseContact * gwc = static_cast< GroupWiseContact *>( contact );
			static_cast< GroupWiseAccount * >(account())->sendInvitation( m_guid, gwc->dn(), inviteMessage );
		}
	}
}

void GroupWiseChatSession::inviteContact( const QString &contactId )
{
	Kopete::Contact * contact = account()->contacts().value(contactId);
	if ( contact )
		slotInviteContact( contact );
}

void GroupWiseChatSession::slotInviteOtherContact()
{
	if ( !m_searchDlg )
	{
		// show search dialog
		QWidget * w = ( view(false) ? dynamic_cast<KMainWindow*>( view(false)->mainWidget()->topLevelWidget() ) :
					Kopete::UI::Global::mainWidget() );
		m_searchDlg = new KDialog( w);
		m_searchDlg->setCaption(i18n( "Search for Contact to Invite" ));
		m_searchDlg->setButtons(KDialog::Ok|KDialog::Cancel );
		m_searchDlg->setDefaultButton(KDialog::Ok);
		m_search = new GroupWiseContactSearch( account(), QAbstractItemView::SingleSelection, true, m_searchDlg );
		m_searchDlg->setMainWidget( m_search );
		connect( m_search, SIGNAL(selectionValidates(bool)), m_searchDlg, SLOT(enableButtonOk(bool)) );
		m_searchDlg->enableButtonOk( false );
	}
	m_searchDlg->show();
}

void GroupWiseChatSession::slotSearchedForUsers()
{
	// create an item for each result, in the block list
	QList< ContactDetails > selected = m_search->selectedResults();
	if ( selected.count() )
	{
		QWidget * w = ( view(false) ? dynamic_cast<KMainWindow*>( view(false)->mainWidget()->topLevelWidget() ) :
				Kopete::UI::Global::mainWidget() );
		ContactDetails cd = selected.first();
		bool ok;
		QRegExp rx( ".*" );
		QRegExpValidator validator( rx, this );
		QString inviteMessage = KInputDialog::getText( i18n( "Enter Invitation Message" ),
		    i18n( "Enter the reason for the invitation, or leave blank for no reason:" ), QString(), &ok, w, &validator );
		if ( ok )
		{
			account()->sendInvitation( m_guid, cd.dn, inviteMessage );
		}
	}
}

void GroupWiseChatSession::addInvitee( const Kopete::Contact * c )
{
	// create a placeholder contact for each invitee
	kDebug () ;
	QString pending = i18nc("label attached to contacts who have been invited but are yet to join a chat", "(pending)");
	Kopete::MetaContact * inviteeMC = new Kopete::MetaContact();
	inviteeMC->setDisplayName( c->metaContact()->displayName() + pending );
	GroupWiseContact * invitee = new GroupWiseContact( account(), c->contactId() + ' ' + pending, inviteeMC, 0, 0, 0 );
	invitee->setOnlineStatus( c->onlineStatus() );
	// TODO: we could set all the placeholder's properties etc here too
	addContact( invitee, true );
	m_invitees.append( invitee );
}

void GroupWiseChatSession::joined( GroupWiseContact * c )
{
	// we add the real contact before removing the placeholder,
	// because otherwise KMM will delete itself when the last member leaves.
	addContact( c );

	// look for the invitee and remove it
	Kopete::Contact * pending = 0;
	foreach( pending, m_invitees )
	{
		if ( pending->contactId().startsWith( c->contactId() ) )
		{
			removeContact( pending, QString(), Qt::PlainText, true );
			break;
		}
	}

	m_invitees.removeAll( pending );
	delete pending;

	updateArchiving();

	++m_memberCount;
}

void GroupWiseChatSession::left( GroupWiseContact * c )
{
	kDebug() ;
	removeContact( c );
	--m_memberCount;

	updateArchiving();

	if ( m_memberCount == 0 )
	{
		if ( m_invitees.count() )
		{
			Kopete::Message failureNotify( myself(), members() );
			failureNotify.setPlainBody( i18n("All the other participants have left, and other invitations are still pending. Your messages will not be delivered until someone else joins the chat.") );
			appendMessage( failureNotify );
		}
		else
			setClosed();
	}
}

void GroupWiseChatSession::inviteDeclined( GroupWiseContact * c )
{
	// look for the invitee and remove it
	Kopete::Contact * pending = 0;
	foreach( pending, m_invitees )
	{
		if ( pending->contactId().startsWith( c->contactId() ) )
		{
			removeContact( pending, QString(), Qt::PlainText, true );
			break;
		}
	}
	m_invitees.removeAll( pending );
	delete pending;

	QString from = c->metaContact()->displayName();

	Kopete::Message declined( myself(), members() );
	declined.setPlainBody( i18n("%1 has rejected an invitation to join this conversation.", from ) );
	appendMessage( declined );
}

void GroupWiseChatSession::updateArchiving()
{
	bool archiving = false;
	foreach( Kopete::Contact * c, members() )
	{
		GroupWiseContact * contact = static_cast<GroupWiseContact*>( c );
		if ( contact->archiving() )
		{
			archiving = true;
			break;
		}
	}
	if ( archiving )
	{
		m_logging->setEnabled( true );
		m_logging->setToolTip( i18n( "Conversation is being administratively logged" ) );
	}
	else
	{
		m_logging->setEnabled( false );
		m_logging->setToolTip( i18n( "Conversation is not being administratively logged" ) );
	}
}

void GroupWiseChatSession::slotShowSecurity()
{
	QWidget * w = ( view(false) ? dynamic_cast<KMainWindow*>( view(false)->mainWidget()->topLevelWidget() ) :
				Kopete::UI::Global::mainWidget() );
	KMessageBox::queuedMessageBox( w, KMessageBox::Information, i18n( "This conversation is secured with SSL security." ), i18n("Security Status" ) );
}

void GroupWiseChatSession::slotShowArchiving()
{
	QWidget * w = ( view(false) ? dynamic_cast<KMainWindow*>( view(false)->mainWidget()->topLevelWidget() ) :
				Kopete::UI::Global::mainWidget() );
	KMessageBox::queuedMessageBox( w, KMessageBox::Information, i18n( "This conversation is being logged administratively." ), i18n("Archiving Status" ) );
}

#include "gwmessagemanager.moc"
