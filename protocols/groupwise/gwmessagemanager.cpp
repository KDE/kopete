//
// C++ Implementation: gwmessagemanager
//
// Description: 
//
//
// Author: SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <qlabel.h>
#include <qvalidator.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kshortcut.h>

#include <kopetecontact.h>
#include <kopetecontactaction.h>
#include <kopetemetacontact.h>
#include <kopetemessagemanagerfactory.h>
#include <kopeteprotocol.h>
#include <kopeteuiglobal.h>
#include <kopeteview.h>

#include "client.h"
#include "gwaccount.h"
#include "gwcontact.h"
#include "gwerror.h"
#include "gwprotocol.h"
#include "gwsearch.h"

#include "gwmessagemanager.h"

void GroupWiseMessageManager::Dict::insert( const ConferenceGuid & key, GroupWiseMessageManager * item )
{
	QMap< ConferenceGuid, GroupWiseMessageManager * >::insert( key.left( CONF_GUID_END ), item  );
}

GroupWiseMessageManager * GroupWiseMessageManager::Dict::operator[]( const ConferenceGuid & key )
{
	return QMap< ConferenceGuid, GroupWiseMessageManager * >::operator[]( key.left( CONF_GUID_END ) );
}

void GroupWiseMessageManager::Dict::remove( const ConferenceGuid & key )
{
	QMap< ConferenceGuid, GroupWiseMessageManager * >::remove( key.left( CONF_GUID_END ) );
}

GroupWiseMessageManager::GroupWiseMessageManager(const KopeteContact* user, KopeteContactPtrList others, KopeteProtocol* protocol, const GroupWise::ConferenceGuid & guid, int id, const char* name): KopeteMessageManager(user, others, protocol, 0, name), m_guid( guid ), m_flags( 0 ), m_searchDlg( 0 ), m_memberCount( others.count() )
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "New message manager for " << user->contactId() << endl;

	// Needed because this is (indirectly) a KXMLGuiClient, so it can find the gui description .rc file
	setInstance( protocol->instance() );
	
	// make sure Kopete knows about this instance
	KopeteMessageManagerFactory::factory()->addKopeteMessageManager ( this );

	connect ( this, SIGNAL( messageSent ( KopeteMessage &, KopeteMessageManager * ) ),
			  SLOT( slotMessageSent ( KopeteMessage &, KopeteMessageManager * ) ) );
	connect( this, SIGNAL( typingMsg ( bool ) ), SLOT( slotSendTypingNotification ( bool ) ) );
	connect( account(), SIGNAL( contactTyping( const ConferenceEvent & ) ), 
						SLOT( slotGotTypingNotification( const ConferenceEvent & ) ) );
	connect( account(), SIGNAL( contactNotTyping( const ConferenceEvent & ) ), 
						SLOT( slotGotNotTypingNotification( const ConferenceEvent & ) ) );
	
	// Set up the Invite menu
	m_actionInvite = new KActionMenu( i18n( "&Invite" ), actionCollection() , "gwInvite" );
	connect( m_actionInvite->popupMenu(), SIGNAL( aboutToShow() ), this, SLOT(slotActionInviteAboutToShow() ) ) ;
	
	m_secure = new KAction( i18n( "Security Status" ), "encrypted", KShortcut(), this, SLOT( slotShowSecurity() ), actionCollection(), "gwSecureChat" );
	m_secure->setToolTip( i18n( "Conversation is secure" ) );
	
	m_logging = new KAction( i18n( " Archiving Status" ), "logchat", KShortcut(), this, SLOT( slotShowArchiving() ), actionCollection(), "gwLoggingChat" );
	updateArchiving();
	
	setXMLFile("gwchatui.rc");
	setMayInvite( true );

	m_invitees.setAutoDelete( true );
}

GroupWiseMessageManager::~GroupWiseMessageManager()
{
}

void GroupWiseMessageManager::setGuid( const GroupWise::ConferenceGuid & guid )
{
	if ( m_guid.isEmpty() )
	{
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "setting GUID to: " << guid << endl;
		m_guid = guid;
	}
	else
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "attempted to change the conference's GUID when already set!" << endl;
}

bool GroupWiseMessageManager::closed()
{
	return m_flags & GroupWise::Closed;
}

bool GroupWiseMessageManager::logging()
{
	return m_flags & GroupWise::Logging;
}

bool GroupWiseMessageManager::secure()
{
	return m_flags & GroupWise::Secure;
}

void GroupWiseMessageManager::setClosed()
{
	m_flags = m_flags | GroupWise::Closed;
}

void GroupWiseMessageManager::setLogging( bool logging )
{
	if ( logging )
		m_flags = m_flags | GroupWise::Logging;
	else
		m_flags = m_flags & !GroupWise::Logging;
}

void GroupWiseMessageManager::setSecure( bool secure )
{
	if ( secure )
		m_flags = m_flags | GroupWise::Secure;
	else
		m_flags = m_flags & !GroupWise::Secure;
}

GroupWiseAccount *  GroupWiseMessageManager::account()
{
	return static_cast<GroupWiseAccount *>( KopeteMessageManager::account() );
}

void GroupWiseMessageManager::createConference()
{
	if ( m_guid.isEmpty() )
	{
		kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
		// form a list of invitees
		QStringList invitees;
		KopeteContactPtrList chatMembers = members();
		for ( KopeteContact * contact = chatMembers.first(); contact; contact = chatMembers.next() )
		{
			invitees.append( static_cast< GroupWiseContact * >( contact )->dn() );
		}
		// this is where we will set the GUID and send any pending messages
		connect( account(), SIGNAL( conferenceCreated( const int, const GroupWise::ConferenceGuid & ) ), SLOT( receiveGuid( const int, const GroupWise::ConferenceGuid & ) ) );
		connect( account(), SIGNAL( conferenceCreationFailed( const int, const int ) ), SLOT( slotCreationFailed( const int, const int ) ) );
		
		// create the conference
		account()->createConference( mmId(), invitees );
	}
	else
		kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << " tried to create conference on the server when it was already instantiated" << endl;
}

void GroupWiseMessageManager::receiveGuid( const int newMmId, const GroupWise::ConferenceGuid & guid )
{
	if ( newMmId == mmId() )
	{
		kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << " got GUID from server" << endl;
		m_memberCount = members().count();
		setGuid( guid );
		// re-add all the members.  This is because when the last member leaves the conference, 
		// they are removed from the chat member list GUI.  By re-adding them here, we guarantee they appear
		// in the UI again, at the price of a debug message when starting up a new chatwindow
		
		QPtrListIterator< KopeteContact > it( members() );
		KopeteContact * contact;
		while ( ( contact = it.current() ) )
		{
			++it;
			addContact( contact, true );
		}
		
		// notify the contact(s) using this message manager that it's been instantiated on the server
		emit conferenceCreated();
		// TODO: send invitations if we're not inviting in the conf create...
		dequeueMessagesAndInvites();
	}
}

void GroupWiseMessageManager::slotCreationFailed( const int failedId, const int statusCode )
{
	if ( failedId == mmId() )
	{
		kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << " couldn't start a chat, no GUID.\n" << endl;
		//emit creationFailed();
		KopeteMessage failureNotify = KopeteMessage( user(), members(), i18n("An error occurred when trying to start a chat: %1").arg( statusCode ), KopeteMessage::Internal, KopeteMessage::PlainText);
		appendMessage( failureNotify );
		setClosed();
	}
}

void GroupWiseMessageManager::slotSendTypingNotification( bool typing )
{
	// only send a notification if we've got a conference going and we are not Appear Offline
	if ( !m_guid.isEmpty() && m_memberCount &&
		  ( account()->myself()->onlineStatus() != GroupWiseProtocol::protocol()->groupwiseAppearOffline ) )
				account()->client()->sendTyping( guid(), typing );
}

void GroupWiseMessageManager::slotMessageSent( KopeteMessage & message, KopeteMessageManager * )
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	if( account()->isConnected() )
	{
		if ( closed() )
		{
			KopeteMessage failureNotify = KopeteMessage( user(), members(), i18n("Your message could not be sent. This conversation has been closed by the server, because all the other participants left or declined invitations. "), KopeteMessage::Internal, KopeteMessage::PlainText);
			appendMessage( failureNotify );
			messageSucceeded();
		}
		else if ( account()->myself()->onlineStatus() == ( static_cast<GroupWiseProtocol *>( protocol() ) )->groupwiseAppearOffline )
		{
			KopeteMessage failureNotify = KopeteMessage( user(), members(), i18n("Your message could not be sent. You cannot send messages while your status is Appear Offline. "), KopeteMessage::Internal, KopeteMessage::PlainText);
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
					// the message won't go anywhere, as there's noone there except invitees, but we warn the user
					// when the last participant leaves.
					messageSucceeded();
				}
				else
				{
					kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << "waiting for server to create a conference, queuing message" << endl;
					// the conference hasn't been instantiated on the server yet, so queue the message
					m_guid = ConferenceGuid();
					createConference();
					m_pendingOutgoingMessages.append( message );
				}
			}
			else 
			{
				kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << "sending message" << endl;
				account()->sendMessage( guid(), message );
				// we could wait until the server acks our send, 
				// but we'd need a UID for outgoing messages and a list to track them
				appendMessage( message );
				messageSucceeded();
			}
		}
	}
}

void GroupWiseMessageManager::slotGotTypingNotification( const ConferenceEvent& event )
{
	if ( event.guid == guid() )
		receivedTypingMsg( static_cast<GroupWiseProtocol *>( protocol() )->dnToDotted( event.user ), true );
}

void GroupWiseMessageManager::slotGotNotTypingNotification( const ConferenceEvent& event )
{
	if ( event.guid == guid() )
		receivedTypingMsg( static_cast<GroupWiseProtocol *>( protocol() )->dnToDotted( event.user ), false );
}

void GroupWiseMessageManager::dequeueMessagesAndInvites()
{
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	for ( QValueListIterator< KopeteMessage > it = m_pendingOutgoingMessages.begin();
		  it != m_pendingOutgoingMessages.end();
		  ++it )
	{
		slotMessageSent( *it, this );
	}
	m_pendingOutgoingMessages.clear();
	QPtrListIterator< KopeteContact > it( m_pendingInvites );
	KopeteContact * contact;
	while ( ( contact = it.current() ) )
	{
		++it;
		slotInviteContact( contact );
	}
	m_pendingInvites.clear();
}

void GroupWiseMessageManager::slotActionInviteAboutToShow()
{
	// We can't simply insert  KAction in this menu bebause we don't know when to delete them.
	//  items inserted with insert items are automatically deleted when we call clear

	m_inviteActions.setAutoDelete(true);
	m_inviteActions.clear();

	m_actionInvite->popupMenu()->clear();

	
	QDictIterator<KopeteContact> it( account()->contacts() );
	for( ; it.current(); ++it )
	{
		if( !members().contains( it.current() ) && it.current()->isOnline() && it.current() != user() )
		{
			KAction *a=new KopeteContactAction( it.current(), this,
				SLOT( slotInviteContact( KopeteContact * ) ), m_actionInvite );
			m_actionInvite->insert( a );
			m_inviteActions.append( a ) ;
		}
	}
	// Invite someone off-list
	KAction *b=new KAction( i18n ("&Other..."), 0, this, SLOT( slotInviteOtherContact() ), m_actionInvite, "actionOther" );
	m_actionInvite->insert( b );
	m_inviteActions.append( b ) ;
}

void GroupWiseMessageManager::slotInviteContact( KopeteContact * contact )
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
				i18n( "Enter the reason for the invitation, or leave blank for no reason" ), QString(),
				&ok, w ? w : Kopete::UI::Global::mainWidget(), "invitemessagedlg", &validator );
		if ( ok )
		{	
			GroupWiseContact * gwc = static_cast< GroupWiseContact *>( contact );
			static_cast< GroupWiseAccount * >(account())->sendInvitation( m_guid, gwc->dn(), inviteMessage );
		}
	}
}

void GroupWiseMessageManager::inviteContact( const QString &contactId )
{
	KopeteContact * contact = account()->contacts()[ contactId ];
	if ( contact )
		slotInviteContact( contact );
}

void GroupWiseMessageManager::slotInviteOtherContact()
{
	if ( !m_searchDlg )
	{
		// show search dialog
		QWidget * w = ( view(false) ? dynamic_cast<KMainWindow*>( view(false)->mainWidget()->topLevelWidget() ) : 
					Kopete::UI::Global::mainWidget() );
		m_searchDlg = new KDialogBase( w, "invitesearchdialog", false, i18n( "Search for contact to invite" ), KDialogBase::Ok|KDialogBase::Cancel|KDialogBase::User1, KDialogBase::User1, true, KGuiItem( i18n( "&Search" ) ) );
		m_search = new GroupWiseSearch( account(), QListView::Single, true, m_searchDlg, "invitesearchwidget" );
		m_searchDlg->setMainWidget( m_search );
		connect( m_searchDlg, SIGNAL( okClicked() ), SLOT( slotSearchedForUsers() ) );
		connect( m_searchDlg, SIGNAL( user1Clicked() ), m_search, SLOT( doSearch() ) );
		connect( m_search, SIGNAL( selectionValidates( bool ) ), m_searchDlg, SLOT( enableButtonOK( bool ) ) );
		m_searchDlg->enableButtonOK( false );
	}
	m_searchDlg->show();
}

void GroupWiseMessageManager::slotSearchedForUsers()
{
	// create an item for each result, in the block list
	QValueList< ContactDetails > selected = m_search->selectedResults();
	if ( selected.count() )
	{
		QWidget * w = ( view(false) ? dynamic_cast<KMainWindow*>( view(false)->mainWidget()->topLevelWidget() ) :
				Kopete::UI::Global::mainWidget() ); 
		ContactDetails cd = selected.first();
		bool ok;
		QRegExp rx( ".*" );
		QRegExpValidator validator( rx, this );
		QString inviteMessage = KInputDialog::getText( i18n( "Enter Invitation Message" ),
				i18n( "Enter the reason for the invitation, or leave blank for no reason" ), QString(),
				&ok, w , "invitemessagedlg", &validator );
		if ( ok )
		{	
			account()->sendInvitation( m_guid, cd.dn, inviteMessage );
		}
	}
}

void GroupWiseMessageManager::addInvitee( const KopeteContact * c )
{
	// create a placeholder contact for each invitee
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	QString pending = i18n("label attached to contacts who have been invited but are yet to join a chat", "(pending)");
	KopeteMetaContact * inviteeMC = new KopeteMetaContact();
	inviteeMC->setDisplayName( c->metaContact()->displayName() + pending );
	GroupWiseContact * invitee = new GroupWiseContact( account(), c->contactId() + " " + pending, inviteeMC, 0, 0, 0 );
	invitee->setOnlineStatus( c->onlineStatus() );
	// TODO: we could set all the placeholder's properties etc here too
	addContact( invitee, true );
	m_invitees.append( invitee );
}

void GroupWiseMessageManager::joined( GroupWiseContact * c )
{
	// look for the invitee and remove it
	KopeteContact * pending;
	for ( pending = m_invitees.first(); pending; pending = m_invitees.next() )
	{
		if ( pending->contactId().startsWith( c->contactId() ) )
		{
			removeContact( pending, QString::null, KopeteMessage::PlainText, true );
			break;
		}
	}

	m_invitees.remove( pending );

	addContact( c );

	c->joinConference( m_guid );
	
	updateArchiving();
	
	++m_memberCount;
}

void GroupWiseMessageManager::left( GroupWiseContact * c )
{
	removeContact( c );
	c->leaveConference( m_guid );
	--m_memberCount;		
	
	updateArchiving();
	
	if ( m_memberCount == 0 )
	{
		if ( m_invitees.count() )
		{
			KopeteMessage failureNotify = KopeteMessage( user(), members(), 
						i18n("All the other participants have left, and other invitations are still pending. Your messages will not be delivered until someone else joins the chat."), 
						KopeteMessage::Internal, KopeteMessage::PlainText );
			appendMessage( failureNotify );
		}
		else
			setClosed();
	}
}

void GroupWiseMessageManager::inviteDeclined( GroupWiseContact * c )
{
	// look for the invitee and remove it
	KopeteContact * pending;
	for ( pending = m_invitees.first(); pending; pending = m_invitees.next() )
	{
		if ( pending->contactId().startsWith( c->contactId() ) )
		{
			removeContact( pending, QString::null, KopeteMessage::PlainText, true );
			break;
		}
	}
	m_invitees.remove( pending );
	
	QString from = c->metaContact()->displayName();

	KopeteMessage declined = KopeteMessage( user(), members(), 
				i18n("%1 has rejected an invitation to join this conversation.").arg( from ), 
				KopeteMessage::Internal, KopeteMessage::PlainText );
	appendMessage( declined );
}

void GroupWiseMessageManager::updateArchiving()
{
	bool archiving = false;
	QPtrListIterator< KopeteContact > it( members() );
	GroupWiseContact * contact;
	while ( ( contact = static_cast<GroupWiseContact*>( it.current() ) ) )
	{
		++it;
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

void GroupWiseMessageManager::slotShowSecurity()
{
	QWidget * w = ( view(false) ? dynamic_cast<KMainWindow*>( view(false)->mainWidget()->topLevelWidget() ) :
				Kopete::UI::Global::mainWidget() ); 
	KMessageBox::queuedMessageBox( w, KMessageBox::Information, i18n( "This conversation is secured with SSL security." ), i18n("Security Status" ) );
}

void GroupWiseMessageManager::slotShowArchiving()
{
	QWidget * w = ( view(false) ? dynamic_cast<KMainWindow*>( view(false)->mainWidget()->topLevelWidget() ) :
				Kopete::UI::Global::mainWidget() ); 
	KMessageBox::queuedMessageBox( w, KMessageBox::Information, i18n( "This conversation is being logged administratively." ), i18n("Archiving Status" ) );
}

#include "gwmessagemanager.moc"
