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

#include "gwmessagemanager.h"

GroupWiseChatSession::GroupWiseChatSession(const Kopete::Contact* user, Kopete::ContactPtrList others, Kopete::Protocol* protocol, const GroupWise::ConferenceGuid & guid, int id, const char* name): Kopete::ChatSession(user, others, protocol, name), m_guid( guid ), m_flags( 0 ), m_searchDlg( 0 ), m_memberCount( others.count() )
{
	Q_UNUSED( id );
	static uint s_id=0;
	m_mmId=++s_id;
	
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "New message manager for " << user->contactId() << endl;

	// Needed because this is (indirectly) a KXMLGuiClient, so it can find the gui description .rc file
	setInstance( protocol->instance() );
	
	// make sure Kopete knows about this instance
	Kopete::ChatSessionManager::self()->registerChatSession ( this );

	connect ( this, SIGNAL( messageSent ( Kopete::Message &, Kopete::ChatSession * ) ),
			  SLOT( slotMessageSent ( Kopete::Message &, Kopete::ChatSession * ) ) );
	connect( this, SIGNAL( myselfTyping ( bool ) ), SLOT( slotSendTypingNotification ( bool ) ) );
	connect( account(), SIGNAL( contactTyping( const ConferenceEvent & ) ), 
						SLOT( slotGotTypingNotification( const ConferenceEvent & ) ) );
	connect( account(), SIGNAL( contactNotTyping( const ConferenceEvent & ) ), 
						SLOT( slotGotNotTypingNotification( const ConferenceEvent & ) ) );
	
	// Set up the Invite menu
	m_actionInvite = new KActionMenu( i18n( "&Invite" ), actionCollection() , "gwInvite" );
	connect( m_actionInvite->popupMenu(), SIGNAL( aboutToShow() ), this, SLOT(slotActionInviteAboutToShow() ) ) ;
	
	m_secure = new KAction( i18n( "Security Status" ), "encrypted", KShortcut(), this, SLOT( slotShowSecurity() ), actionCollection(), "gwSecureChat" );
	m_secure->setToolTip( i18n( "Conversation is secure" ) );
	
	m_logging = new KAction( i18n( "Archiving Status" ), "logchat", KShortcut(), this, SLOT( slotShowArchiving() ), actionCollection(), "gwLoggingChat" );
	updateArchiving();
	
	setXMLFile("gwchatui.rc");
	setMayInvite( true );

	m_invitees.setAutoDelete( true );
}

GroupWiseChatSession::~GroupWiseChatSession()
{
	emit leavingConference( this );
}

uint GroupWiseChatSession::mmId() const
{
  return m_mmId;
}

void GroupWiseChatSession::setGuid( const GroupWise::ConferenceGuid & guid )
{
	if ( m_guid.isEmpty() )
	{
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "setting GUID to: " << guid << endl;
		m_guid = guid;
	}
	else
		kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "attempted to change the conference's GUID when already set!" << endl;
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
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << " Conference " << m_guid << " is now Closed " << endl;
	m_guid = QString::null;
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
		kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
		// form a list of invitees
		QStringList invitees;
		Kopete::ContactPtrList chatMembers = members();
		for ( Kopete::Contact * contact = chatMembers.first(); contact; contact = chatMembers.next() )
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

void GroupWiseChatSession::receiveGuid( const int newMmId, const GroupWise::ConferenceGuid & guid )
{
	if ( newMmId == mmId() )
	{
		kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << " got GUID from server" << endl;
		m_memberCount = members().count();
		setGuid( guid );
		// re-add all the members.  This is because when the last member leaves the conference, 
		// they are removed from the chat member list GUI.  By re-adding them here, we guarantee they appear
		// in the UI again, at the price of a debug message when starting up a new chatwindow
		
		QPtrListIterator< Kopete::Contact > it( members() );
		Kopete::Contact * contact;
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

void GroupWiseChatSession::slotCreationFailed( const int failedId, const int statusCode )
{
	if ( failedId == mmId() )
	{
		kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << " couldn't start a chat, no GUID.\n" << endl;
		//emit creationFailed();
		Kopete::Message failureNotify = Kopete::Message( myself(), members(), i18n("An error occurred when trying to start a chat: %1").arg( statusCode ), Kopete::Message::Internal, Kopete::Message::PlainText);
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
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
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
			Kopete::Message failureNotify = Kopete::Message( myself(), members(), i18n("Your message could not be sent. You cannot send messages while your status is Appear Offline. "), Kopete::Message::Internal, Kopete::Message::PlainText);
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
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	for ( QValueListIterator< Kopete::Message > it = m_pendingOutgoingMessages.begin();
		  it != m_pendingOutgoingMessages.end();
		  ++it )
	{
		slotMessageSent( *it, this );
	}
	m_pendingOutgoingMessages.clear();
	QPtrListIterator< Kopete::Contact > it( m_pendingInvites );
	Kopete::Contact * contact;
	while ( ( contact = it.current() ) )
	{
		++it;
		slotInviteContact( contact );
	}
	m_pendingInvites.clear();
}

void GroupWiseChatSession::slotActionInviteAboutToShow()
{
	// We can't simply insert  KAction in this menu bebause we don't know when to delete them.
	//  items inserted with insert items are automatically deleted when we call clear

	m_inviteActions.setAutoDelete(true);
	m_inviteActions.clear();

	m_actionInvite->popupMenu()->clear();

	
	QDictIterator<Kopete::Contact> it( account()->contacts() );
	for( ; it.current(); ++it )
	{
		if( !members().contains( it.current() ) && it.current()->isOnline() && it.current() != myself() )
		{
			KAction *a=new KopeteContactAction( it.current(), this,
				SLOT( slotInviteContact( Kopete::Contact * ) ), m_actionInvite );
			m_actionInvite->insert( a );
			m_inviteActions.append( a ) ;
		}
	}
	// Invite someone off-list
	KAction *b=new KAction( i18n ("&Other..."), 0, this, SLOT( slotInviteOtherContact() ), m_actionInvite, "actionOther" );
	m_actionInvite->insert( b );
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
		    i18n( "Enter the reason for the invitation, or leave blank for no reason:" ), QString(),
				&ok, w ? w : Kopete::UI::Global::mainWidget(), "invitemessagedlg", &validator );
		if ( ok )
		{	
			GroupWiseContact * gwc = static_cast< GroupWiseContact *>( contact );
			static_cast< GroupWiseAccount * >(account())->sendInvitation( m_guid, gwc->dn(), inviteMessage );
		}
	}
}

void GroupWiseChatSession::inviteContact( const QString &contactId )
{
	Kopete::Contact * contact = account()->contacts()[ contactId ];
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
		m_searchDlg = new KDialogBase( w, "invitesearchdialog", false, i18n( "Search for Contact to Invite" ), KDialogBase::Ok|KDialogBase::Cancel );
		m_search = new GroupWiseContactSearch( account(), QListView::Single, true, m_searchDlg, "invitesearchwidget" );
		m_searchDlg->setMainWidget( m_search );
		connect( m_search, SIGNAL( selectionValidates( bool ) ), m_searchDlg, SLOT( enableButtonOK( bool ) ) );
		m_searchDlg->enableButtonOK( false );
	}
	m_searchDlg->show();
}

void GroupWiseChatSession::slotSearchedForUsers()
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
		    i18n( "Enter the reason for the invitation, or leave blank for no reason:" ), QString(),
				&ok, w , "invitemessagedlg", &validator );
		if ( ok )
		{	
			account()->sendInvitation( m_guid, cd.dn, inviteMessage );
		}
	}
}

void GroupWiseChatSession::addInvitee( const Kopete::Contact * c )
{
	// create a placeholder contact for each invitee
	kdDebug ( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	QString pending = i18n("label attached to contacts who have been invited but are yet to join a chat", "(pending)");
	Kopete::MetaContact * inviteeMC = new Kopete::MetaContact();
	inviteeMC->setDisplayName( c->metaContact()->displayName() + pending );
	GroupWiseContact * invitee = new GroupWiseContact( account(), c->contactId() + " " + pending, inviteeMC, 0, 0, 0 );
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
	Kopete::Contact * pending;
	for ( pending = m_invitees.first(); pending; pending = m_invitees.next() )
	{
		if ( pending->contactId().startsWith( c->contactId() ) )
		{
			removeContact( pending, QString::null, Kopete::Message::PlainText, true );
			break;
		}
	}

	m_invitees.remove( pending );

	updateArchiving();
	
	++m_memberCount;
}

void GroupWiseChatSession::left( GroupWiseContact * c )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;
	removeContact( c );
	--m_memberCount;
	
	updateArchiving();
	
	if ( m_memberCount == 0 )
	{
		if ( m_invitees.count() )
		{
			Kopete::Message failureNotify = Kopete::Message( myself(), members(), 
						i18n("All the other participants have left, and other invitations are still pending. Your messages will not be delivered until someone else joins the chat."), 
						Kopete::Message::Internal, Kopete::Message::PlainText );
			appendMessage( failureNotify );
		}
		else
			setClosed();
	}
}

void GroupWiseChatSession::inviteDeclined( GroupWiseContact * c )
{
	// look for the invitee and remove it
	Kopete::Contact * pending;
	for ( pending = m_invitees.first(); pending; pending = m_invitees.next() )
	{
		if ( pending->contactId().startsWith( c->contactId() ) )
		{
			removeContact( pending, QString::null, Kopete::Message::PlainText, true );
			break;
		}
	}
	m_invitees.remove( pending );
	
	QString from = c->metaContact()->displayName();

	Kopete::Message declined = Kopete::Message( myself(), members(), 
				i18n("%1 has rejected an invitation to join this conversation.").arg( from ), 
				Kopete::Message::Internal, Kopete::Message::PlainText );
	appendMessage( declined );
}

void GroupWiseChatSession::updateArchiving()
{
	bool archiving = false;
	QPtrListIterator< Kopete::Contact > it( members() );
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
