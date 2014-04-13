//
// C++ Implementation: qqchatmessager
//
// Description:
//
//
// Author: Hui Jin
// based on qqmessagemanager by SUSE AG <>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "qqchatsession.h"

#include <qlabel.h>
#include <qvalidator.h>
#include <QList>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kxmlguiwindow.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kactionmenu.h>
#include <kshortcut.h>
#include <kicon.h>

#include <kopetecontact.h>
#include <kopetecontactaction.h>
#include <kopetemetacontact.h>
#include <kopetechatsessionmanager.h>
#include <kopeteprotocol.h>
#include <kopeteuiglobal.h>
#include <kopeteview.h>

#include "qqaccount.h"
#include "qqcontact.h"
#include "qqprotocol.h"
#include <kactioncollection.h>

QQChatSession::QQChatSession( const Kopete::Contact* user, Kopete::ContactPtrList others, Kopete::Protocol* protocol, const QString& guid) : Kopete::ChatSession(user, others, protocol), m_guid( guid ), m_flags( 0 ), m_searchDlg( 0 ), m_memberCount( others.count() )
{
	static int s_id=0;
	m_mmId=++s_id;

	kDebug ( 14140 ) << "New message manager for " << user->contactId();

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
        actionCollection()->addAction( "qqInvite", m_actionInvite );
	connect( m_actionInvite->menu(), SIGNAL(aboutToShow()), this, SLOT(slotActionInviteAboutToShow()) ) ;

	m_secure = actionCollection()->addAction( "qqSecureChat" );
	m_secure->setText( i18n( "Security Status" ) );
        m_secure->setIcon( KIcon( "security-high" ) );
	m_secure->setToolTip( i18n( "Conversation is secure" ) );
        connect( m_secure, SIGNAL(triggered()), this, SLOT(slotShowSecurity()) );

	m_logging = actionCollection()->addAction( "qqLoggingChat" );
	m_logging->setText( i18n( "Archiving Status" ) );
        m_logging->setIcon( KIcon( "utilities-log-viewer" ) );
        connect( m_logging, SIGNAL(triggered()), this, SLOT(slotShowArchiving()) );
	updateArchiving();

	setXMLFile("qqchatui.rc");
	setMayInvite( true );

	// FIXME: do we need to delete it in the destructor ?
	// m_invitees.setAutoDelete( true );
}

QQChatSession::~QQChatSession()
{
	emit leavingConference( this );
}

int QQChatSession::mmId() const
{
  return m_mmId;
}

void QQChatSession::setGuid( const QString& guid )
{
	if ( m_guid.isEmpty() )
	{
		kDebug( 14140 ) << "setting GUID to: " << guid;
		m_guid = guid;
	}
	else
		kDebug( 14140 ) << "attempted to change the conference's GUID when already set!";
}

void QQChatSession::setClosed()
{
	kDebug( 14140 ) << " Conference " << m_guid << " is now Closed ";
	m_guid.clear();
}


QQAccount *  QQChatSession::account()
{
	return static_cast<QQAccount *>( Kopete::ChatSession::account() );
}

void QQChatSession::createConference()
{
	if ( m_guid.isEmpty() )
	{
		kDebug ( 14140 ) ;
		// form a list of invitees
		QStringList invitees;
		Kopete::ContactPtrList chatMembers = members();
		Kopete::ContactPtrList::const_iterator contact;
		for ( contact = chatMembers.constBegin(); contact != chatMembers.constEnd(); contact++ )
		{
			// FIXME: We don't have dn, just use the contactId()
			invitees.append( static_cast<QQContact*>( *contact )->contactId() );
		}
		// this is where we will set the GUID and send any pending messages
		connect( account(), SIGNAL(conferenceCreated(int,QString)), SLOT(receiveGuid(int,QString)) );
		connect( account(), SIGNAL(conferenceCreationFailed(int,int)), SLOT(slotCreationFailed(int,int)) );

		// create the conference
		// account()->createConference( mmId(), invitees );
	}
	else
		kDebug ( 14140 ) << " tried to create conference on the server when it was already instantiated";
}

void QQChatSession::receiveGuid( const int newMmId, const QString & guid )
{
	if ( newMmId == mmId() )
	{
		kDebug ( 14140 ) << " got GUID from server";
		m_memberCount = members().count();
		setGuid( guid );
		// re-add all the members.  This is because when the last member leaves the conference,
		// they are removed from the chat member list GUI.  By re-adding them here, we guarantee they appear
		// in the UI again, at the price of a debug message when starting up a new chatwindow
		Kopete::ContactPtrList chatMembers = members();
		Kopete::ContactPtrList::const_iterator contact;
		for ( contact = chatMembers.constBegin(); contact != chatMembers.constEnd(); contact++ )
			addContact( *contact, true );

		// notify the contact(s) using this message manager that it's been instantiated on the server
		emit conferenceCreated();
		// TODO: send invitations if we're not inviting in the conf create...
		dequeueMessagesAndInvites();
	}
}

void QQChatSession::slotCreationFailed( const int failedId, const int statusCode )
{
	if ( failedId == mmId() )
	{
		kDebug ( 14140 ) << " could not start a chat, no GUID.\n";
		//emit creationFailed();
		Kopete::Message failureNotify( myself(), members() );
		failureNotify.setPlainBody( i18n("An error occurred when trying to start a chat: %1", statusCode ) );
		failureNotify.setDirection( Kopete::Message::Internal );

		appendMessage( failureNotify );
		setClosed();
	}
}

void QQChatSession::slotSendTypingNotification( bool typing )
{
	Q_UNUSED(typing);
	// only send a notification if we've got a conference going and we are not Appear Offline
	// TODO: implement me later.
}

void QQChatSession::slotMessageSent( Kopete::Message & message, Kopete::ChatSession * )
{
	kDebug ( 14140 ) ;
	if( account()->isConnected() )
	{
		/*if ( closed() )
		{
			Kopete::Message failureNotify = Kopete::Message( myself(), members(), i18n("Your message could not be sent. This conversation has been closed by the server, because all the other participants left or declined invitations. "), Kopete::Message::Internal, Kopete::Message::PlainText);
			appendMessage( failureNotify );
			messageSucceeded();
		}
		else*/ if ( account()->myself()->onlineStatus() ==  QQProtocol::protocol()->Offline )
		{
			Kopete::Message failureNotify( myself(), members() );
			failureNotify.setPlainBody( i18n("Your message could not be sent. You cannot send messages while your status is Appear Offline. ") );
			failureNotify.setDirection( Kopete::Message::Internal );

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
					kDebug ( 14140 ) << "waiting for server to create a conference, queuing message";
					// the conference hasn't been instantiated on the server yet, so queue the message
					m_guid.clear();
					createConference();
					m_pendingOutgoingMessages.append( message );
				}
			}
			else
			{
				// we are working for this now.
				account()->sendMessage( guid(), message );
				// we could wait until the server acks our send,
				// but we'd need a UID for outgoing messages and a list to track them
				kDebug ( 14140 ) << "sending message: " << message.plainBody();
				appendMessage( message );
				messageSucceeded();
			}
		}
	}
}

void QQChatSession::slotGotTypingNotification( const ConferenceEvent& event )
{
	if ( event.guid == guid() ) {
		// receivedTypingMsg( static_cast<QQProtocol *>( protocol() )->dnToDotted( event.user ), true );
        }
}

void QQChatSession::slotGotNotTypingNotification( const ConferenceEvent& event )
{
	if ( event.guid == guid() ) {
		//receivedTypingMsg( static_cast<QQProtocol *>( protocol() )->dnToDotted( event.user ), false );
        }
}

void QQChatSession::dequeueMessagesAndInvites()
{
	kDebug ( 14140 ) ;
	for ( QList< Kopete::Message >::iterator it = m_pendingOutgoingMessages.begin();
		  it != m_pendingOutgoingMessages.end();
		  ++it )
	{
		slotMessageSent( *it, this );
	}
	m_pendingOutgoingMessages.clear();

	Kopete::ContactPtrList::const_iterator contact;
	for ( contact = m_pendingInvites.constBegin(); contact != m_pendingInvites.constEnd(); contact++ )
		slotInviteContact( *contact );

	m_pendingInvites.clear();
}

void QQChatSession::slotActionInviteAboutToShow()
{
	// We can't simply insert  KAction in this menu bebause we don't know when to delete them.
	//  items inserted with insert items are automatically deleted when we call clear

	qDeleteAll(m_inviteActions);
	m_inviteActions.clear();

	m_actionInvite->menu()->clear();

	QHash<QString, Kopete::Contact*>::const_iterator it;
	for ( it = account()->contacts().constBegin(); it != account()->contacts().constEnd(); it++ )
	{
		if( !members().contains( it.value() ) && it.value()->isOnline() )
		{
			KAction *a = new Kopete::UI::ContactAction( it.value(), actionCollection() );
			m_actionInvite->addAction( a );
			m_inviteActions.append( a ) ;
		}
	}
	// Invite someone off-list
	KAction *b=new KAction( KIcon(), i18n ("&Other..."), actionCollection() );
        actionCollection()->addAction( "actionOther", b );
	QObject::connect( b, SIGNAL(triggered(bool)),
	                  this, SLOT(slotInviteOtherContact()) );
	m_actionInvite->addAction( b );
	m_inviteActions.append( b ) ;
}

void QQChatSession::slotInviteContact( Kopete::Contact * contact )
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
				&ok, w ? w : Kopete::UI::Global::mainWidget(), &validator, QString(), "invitemessagedlg" );
		if ( ok )
		{
			QQContact * qqc = static_cast< QQContact *>( contact );
			static_cast< QQAccount * >(account())->sendInvitation( m_guid, qqc->contactId(), inviteMessage );
		}
	}
}

void QQChatSession::inviteContact( const QString &contactId )
{
	Kopete::Contact * contact = account()->contacts().value( contactId );
	if ( contact )
		slotInviteContact( contact );
}

void QQChatSession::slotInviteOtherContact()
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
		// m_search = new QQContactSearch( account(), Q3ListView::Single, true, m_searchDlg, "invitesearchwidget" );
		// m_searchDlg->setMainWidget( m_search );
		// connect( m_search, SIGNAL(selectionValidates(bool)), m_searchDlg, SLOT(enableButtonOk(bool)) );
		m_searchDlg->enableButtonOk( false );
	}
	m_searchDlg->show();
}

void QQChatSession::slotSearchedForUsers()
{
	// create an item for each result, in the block list
	/*
	Q3ValueList< ContactDetails > selected = m_search->selectedResults();
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
	*/
}

void QQChatSession::addInvitee( const Kopete::Contact * c )
{
	// create a placeholder contact for each invitee
	kDebug ( 14140 ) ;
	QString pending = i18nc("label attached to contacts who have been invited but are yet to join a chat", "(pending)");
	Kopete::MetaContact * inviteeMC = new Kopete::MetaContact();
	inviteeMC->setDisplayName( c->metaContact()->displayName() + pending );
	// QQContact * invitee = new QQContact( account(), c->contactId() + ' ' + pending, inviteeMC, 0, 0, 0 );
	QQContact * invitee = new QQContact( account(), c->contactId() + ' ' + pending, inviteeMC );
	invitee->setOnlineStatus( c->onlineStatus() );
	// TODO: we could set all the placeholder's properties etc here too
	addContact( invitee, true );
	m_invitees.append( invitee );
}

void QQChatSession::joined( QQContact * c )
{
	// we add the real contact before removing the placeholder,
	// because otherwise KMM will delete itself when the last member leaves.
	addContact( c );

	// look for the invitee and remove it
	Kopete::ContactPtrList::iterator pending;
	for ( pending = m_invitees.begin(); pending != m_invitees.end(); pending++ )
	{
		if ( (*pending)->contactId().startsWith( c->contactId() ) )
		{
			removeContact( *pending, QString(), Qt::PlainText, true );
			break;
		}
	}
	m_invitees.erase( pending );

	updateArchiving();

	++m_memberCount;
}

void QQChatSession::left( QQContact * c )
{
	kDebug( 14140 ) ;
	removeContact( c );
	--m_memberCount;

	updateArchiving();

	if ( m_memberCount == 0 )
	{
		if ( m_invitees.count() )
		{
			Kopete::Message failureNotify( myself(), members() );
			failureNotify.setPlainBody( i18n("All the other participants have left, and other invitations are still pending. Your messages will not be delivered until someone else joins the chat.") );
			failureNotify.setDirection( Kopete::Message::Internal );

			appendMessage( failureNotify );
		}
		else
			setClosed();
	}
}

void QQChatSession::inviteDeclined( QQContact * c )
{
	// look for the invitee and remove it
	Kopete::ContactPtrList::iterator pending;
	for ( pending = m_invitees.begin(); pending != m_invitees.end(); pending++ )
	{
		if ( (*pending)->contactId().startsWith( c->contactId() ) )
		{
			removeContact( *pending, QString(), Qt::PlainText, true );
			break;
		}
	}
	m_invitees.erase( pending );

	QString from = c->metaContact()->displayName();

	Kopete::Message declined = Kopete::Message( myself(), members() );
	declined.setPlainBody( i18n("%1 has rejected an invitation to join this conversation.", from ) );
	declined.setDirection( Kopete::Message::Internal );

	appendMessage( declined );
}

void QQChatSession::updateArchiving()
{
	bool archiving = false;

	Kopete::ContactPtrList chatMembers = members();
	Kopete::ContactPtrList::const_iterator contact;
	for ( contact = chatMembers.constBegin(); contact != chatMembers.constEnd(); contact++ )
	{
		// if ( contact->archiving() )
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

void QQChatSession::slotShowSecurity()
{
	QWidget * w = ( view(false) ? dynamic_cast<KMainWindow*>( view(false)->mainWidget()->topLevelWidget() ) :
				Kopete::UI::Global::mainWidget() );
	KMessageBox::queuedMessageBox( w, KMessageBox::Information, i18n( "This conversation is secured with SSL security." ), i18n("Security Status" ) );
}

void QQChatSession::slotShowArchiving()
{
	QWidget * w = ( view(false) ? dynamic_cast<KMainWindow*>( view(false)->mainWidget()->topLevelWidget() ) :
				Kopete::UI::Global::mainWidget() );
	KMessageBox::queuedMessageBox( w, KMessageBox::Information, i18n( "This conversation is being logged administratively." ), i18n("Archiving Status" ) );
}

#include "qqchatsession.moc"
