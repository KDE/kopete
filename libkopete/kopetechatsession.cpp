/*
    kopetechatsession.cpp - Manages all chats

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002      by Daniel Stone           <dstone@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart@kde.org>
    Copyright (c) 2003      by Jason Keirstead        <jason@keirstead.org>
    Copyright (c) 2005      by Michaël Larouche       <larouche@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopetechatsession.h"

#include <qapplication.h>
#include <qfile.h>
#include <qregexp.h>
#include <qpointer.h>

#include <kdebug.h>
#include <kdeversion.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knotification.h>
#include <kstandarddirs.h>

#include "kopeteaccount.h"
#include "kopetebehaviorsettings.h"
#include "kopetecommandhandler.h"
#include "kopetechatsessionmanager.h"
#include "kopetemessagehandlerchain.h"
#include "kopetemetacontact.h"
#include "kopetegroup.h"
#include "kopeteuiglobal.h"
#include "kopeteglobal.h"
#include "kopeteview.h"
#include "kopetecontact.h"
#include "kopetepluginmanager.h"
#include "kopeteprotocol.h"
#include "kopetepicture.h"
#include "kopeteactivenotification.h"

const int CHAIN_COUNT = 3;

class KMMPrivate
{
public:
	Kopete::ContactPtrList contacts;
	const Kopete::Contact *mUser;
	QMap<const Kopete::Contact *, Kopete::OnlineStatus> contactStatus;
	Kopete::ActiveNotifications typingNotifications;
	Kopete::Protocol *mProtocol;
	bool isEmpty;
	bool mCanBeDeleted;
	unsigned int refcount;
	bool customDisplayName;
	QDateTime awayTime;
	QString displayName;
	QString lastUrl;
	KopeteView *view;
	bool mayInvite;
	Kopete::MessageHandlerChain::Ptr chains[CHAIN_COUNT];
	Kopete::ChatSession::Form form;
	bool warnGroupChat;
};

Kopete::ChatSession::ChatSession( const Kopete::Contact *user,
	Kopete::ContactPtrList others, Kopete::Protocol *protocol, Kopete::ChatSession::Form form )
: QObject( user->account())
{
	d = new KMMPrivate;
	d->mUser = user;
	d->mProtocol = protocol;
	d->isEmpty = others.isEmpty();
	d->mCanBeDeleted = true;
	d->refcount = 0;
	d->view = 0L;
	d->customDisplayName = false;
	d->mayInvite = false;
	d->form = form;
	d->warnGroupChat = true;
	if ( !others.isEmpty() ) {
		d->lastUrl = initLastUrl( others.first() );
	}

	for ( int i = 0; others.size() != i; ++i )
		addContact( others[i], true );

	connect( Kopete::PluginManager::self(), SIGNAL(pluginLoaded(Kopete::Plugin*)), this, SLOT(clearChains()) );
	connect( Kopete::PluginManager::self(), SIGNAL(pluginUnloaded(QString)), this, SLOT(clearChains()) );

	connect( user, SIGNAL(contactDestroyed(Kopete::Contact*)), this, SLOT(slotMyselfDestroyed(Kopete::Contact*)) );
	connect( user, SIGNAL(onlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)), this,
		SLOT(slotOnlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)) );

	if( user->metaContact() )
		connect( user->metaContact(), SIGNAL(photoChanged()), this, SIGNAL(photoChanged()) );

	slotUpdateDisplayName();
}

Kopete::ChatSession::~ChatSession()
{
	//for ( Kopete::Contact *c = d->contacts.first(); c; c = d->contacts.next() )
	//	c->setConversations( c->conversations() - 1 );

	if ( !d )
		return;
	d->mCanBeDeleted = false; //prevent double deletion
	Kopete::ChatSessionManager::self()->removeSession( this );
	emit closing( this );
	delete d;
}

void Kopete::ChatSession::slotOnlineStatusChanged( Kopete::Contact *c, const Kopete::OnlineStatus &status, const Kopete::OnlineStatus &oldStatus )
{
	slotUpdateDisplayName();
	emit onlineStatusChanged((Kopete::Contact*)c, status, oldStatus);
}

void Kopete::ChatSession::setContactOnlineStatus( const Kopete::Contact *contact, const Kopete::OnlineStatus &status )
{
	Kopete::OnlineStatus oldStatus = d->contactStatus[ contact ];
	d->contactStatus[ contact ] = status;
	disconnect( contact, SIGNAL(onlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)),
		this, SIGNAL(onlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)) );
	emit onlineStatusChanged( (Kopete::Contact*)contact, status, oldStatus );
}

const Kopete::OnlineStatus Kopete::ChatSession::contactOnlineStatus( const Kopete::Contact *contact ) const
{
	if ( d->contactStatus.contains( contact ) )
		return d->contactStatus[ contact ];

	return contact->onlineStatus();
}

void Kopete::ChatSession::setLastUrl( const QString &verylastUrl )
{
	  d->lastUrl = verylastUrl;
}

const QString Kopete::ChatSession::lastUrl()
{
	return d->lastUrl;
}

const QString Kopete::ChatSession::displayName()
{
	if ( d->displayName.isNull() )
	{
		slotUpdateDisplayName();
	}

	return d->displayName;
}

void Kopete::ChatSession::setDisplayName( const QString &newName )
{
	d->displayName = newName;
	d->customDisplayName = true;
	emit displayNameChanged();
}

void Kopete::ChatSession::slotUpdateDisplayName()
{
	if( d->customDisplayName )
		return;


	//If there is no member yet, don't try to update the display name
	if ( d->contacts.isEmpty() )
		return;

	d->displayName.clear();
	for(int i = 0; i != d->contacts.size(); i++ )
	{
		Kopete::Contact * c = d->contacts[i];
		if(! d->displayName.isNull() )
			d->displayName.append( QString::fromLatin1( ", " ) ) ;

		if ( c->metaContact() )
			d->displayName.append( c->metaContact()->displayName() );
		else
		{
			d->displayName.append( c->displayName() );
		}
	}

	//If we have only 1 contact, add the status of him
	if ( d->contacts.count() == 1 )
	{
		d->displayName.append( QString::fromLatin1( " (%1)" ).arg( d->contacts.first()->onlineStatus().description() ) );
	}

	emit displayNameChanged();
}

const Kopete::ContactPtrList& Kopete::ChatSession::members() const
{
	return d->contacts;
}

const Kopete::Contact* Kopete::ChatSession::myself() const
{
	return d->mUser;
}

Kopete::Protocol* Kopete::ChatSession::protocol() const
{
	return d->mProtocol;
}


#include "kopetemessagehandler.h"
#include "kopetemessageevent.h"

// FIXME: remove this and the friend decl in KMM
class Kopete::TemporaryKMMCallbackAppendMessageHandler : public Kopete::MessageHandler
{
	QPointer<Kopete::ChatSession> manager;
public:
	TemporaryKMMCallbackAppendMessageHandler( Kopete::ChatSession *manager )
	: manager(manager)
	{
	}
	void handleMessage( Kopete::MessageEvent *event )
	{
		Kopete::Message message = event->message();

		if ( manager )
			emit manager->messageAppended( message, manager );

		delete event;
	}
};

class TempFactory : public Kopete::MessageHandlerFactory
{
public:
	Kopete::MessageHandler *create( Kopete::ChatSession *manager, Kopete::Message::MessageDirection )
	{
		return new Kopete::TemporaryKMMCallbackAppendMessageHandler( manager );
	}
	int filterPosition( Kopete::ChatSession *, Kopete::Message::MessageDirection )
	{
		// FIXME: somewhere after everyone else.
		return 100000;
	}
};

void Kopete::ChatSession::clearChains()
{
	for (int i = 0; i < CHAIN_COUNT; i++)
		d->chains[i] = 0;
}

Kopete::MessageHandlerChain::Ptr Kopete::ChatSession::chainForDirection( Kopete::Message::MessageDirection dir )
{
	if( dir < 0 || dir >= CHAIN_COUNT)
		kFatal(14000) << "invalid message direction " << dir;
	if( !d->chains[dir] )
	{
		TempFactory theTempFactory;
		d->chains[dir] = Kopete::MessageHandlerChain::create( this, dir );
	}
	return d->chains[dir];
}

void Kopete::ChatSession::sendMessage( Kopete::Message &message )
{
	message.setManager( this );
	Kopete::Message sentMessage = message;
	if ( !Kopete::CommandHandler::commandHandler()->processMessage( message, this ) )
	{
		emit messageSent( sentMessage, this );
		if ( ( !account()->isAway() || Kopete::BehaviorSettings::self()->enableEventsWhileAway() ) && !account()->isBusy() )
		{
			KNotification::event(QString::fromLatin1( "kopete_outgoing" ),	i18n( "Outgoing Message Sent" ) );
		}
	}
	else
	{
		messageSucceeded();
	}
}

void Kopete::ChatSession::messageSucceeded()
{
	emit messageSuccess();
}

void Kopete::ChatSession::emitNudgeNotification()
{
	if ( !account()->isBusy() )
		KNotification::event( QString::fromLatin1("buzz_nudge"), i18n("A contact sent you a buzz/nudge.") );
}

void Kopete::ChatSession::appendMessage( Kopete::Message &msg )
{
	msg.setManager( this );

	if ( msg.direction() == Kopete::Message::Inbound )
	{
		const QString nick = myself()->displayName();
		if ( Kopete::BehaviorSettings::self()->highlightEnabled() && !nick.isEmpty() )
		{
			const QString nickNameRegExp = QString::fromLatin1( "(^|[\\W])(%1)([\\W]|$)" ).arg( QRegExp::escape( nick ) );
			if ( msg.plainBody().contains( QRegExp( nickNameRegExp, Qt::CaseInsensitive ) ) )
			{
				msg.setImportance( Kopete::Message::Highlight );
			}
		}

		emit messageReceived( msg, this );
	}

	// outbound messages here are ones the user has sent that are now
	// getting reflected back to the chatwindow. they should go down
	// the incoming chain.
	Kopete::Message::MessageDirection chainDirection = msg.direction();
	if( chainDirection == Kopete::Message::Outbound )
		chainDirection = Kopete::Message::Inbound;

	chainForDirection( chainDirection )->processMessage( msg );

	//looking for urls in the message
	urlSearch( msg );
//	emit messageAppended( msg, this );
}

void Kopete::ChatSession::urlSearch( const Kopete::Message &msg )
{
	//if there are any urls in the message
	QStringList lasturls = findUrls(msg);
	if ( !lasturls.empty() ) {
		//set lasturl for message's chatsession
		msg.manager()->setLastUrl( lasturls.last() );
		//saving new url(s) found in message //file named contactId.lasturls.txt in proper folder
		QString urlfilename = Kopete::ChatSession::getUrlsFileName( msg.manager()->members().first() );
		QFile file( urlfilename );
		file.open( QIODevice::Append );
		QTextStream stream( &file );

		for (int i = 0; i < lasturls.size(); ++i)
			stream << lasturls.at(i) << "\n";
		file.close();
	}
}

QStringList Kopete::ChatSession::findUrls(const Kopete::Message &msg )
{
	Kopete::Message message = msg;
	//we check the message for every pattern
	QString tempstr = message.plainBody();
	QStringList regexppatterns = message.regexpPatterns();
	QRegExp linkregexp;
	QMap<int,QString> mapUrl;

	for (int i = 0; i < regexppatterns.size(); ++i) {
	  linkregexp.setPattern(regexppatterns[i]);
	  int pos = 0;
	  while ((pos = linkregexp.indexIn(tempstr, pos)) != -1) {
	    mapUrl.insert(pos,linkregexp.cap(0));
	    pos += linkregexp.matchedLength(); }
	}
	//we use QMap to sort links as they are in the message (if there are many links in one message)
	//lasturllist[0] - is the earliest
	QStringList lasturllist;
	QMapIterator< int, QString > i(mapUrl);
	while (i.hasNext()) { i.next(); lasturllist << i.value(); }
	lasturllist.replaceInStrings(" ", "");
	//add "http://" to link if needed to open it with a browser
	lasturllist.replaceInStrings(QRegExp( regexppatterns[1] ), QLatin1String("\\1http://\\2\\3" ));

	return lasturllist;
}

QString Kopete::ChatSession::initLastUrl( const Kopete::Contact* c )
{
	QString urlfilename = getUrlsFileName(c);
	if ( !urlfilename.isEmpty() )
	{
		QFile file( urlfilename );
		QString lastUrl;
		if ( file.exists() ) {
			if ( file.open(QIODevice::ReadOnly) ) {
				QTextStream stream( &file );
				while ( !stream.atEnd() )
					lastUrl = stream.readLine();
				file.close(); }
			else {
				kDebug(14310) << "cant open lasturls file for " << c->contactId();
			}
		}
		if ( !lastUrl.isEmpty() )
			return lastUrl;
		else return "";
	}
	else
	{
		kDebug(14310) << "cant find lasturls file for " << c->contactId();
		return "";
	}
}

QString Kopete::ChatSession::getUrlsFileName(const Kopete::Contact* c)
{
	QString name = c->protocol()->pluginId().replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) ) +
		QString::fromLatin1( "/" ) +
		c->account()->accountId().replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) ) +
		QString::fromLatin1( "/" ) +
	c->contactId().replace( QRegExp( QString::fromLatin1( "[./~?*]" ) ), QString::fromLatin1( "-" ) ) +
		QString::fromLatin1( ".lasturls" );

	QString filename = KStandardDirs::locateLocal( "data", QString::fromLatin1( "kopete/urls/" ) + name + QString::fromLatin1( ".txt" ) ) ;

	return filename;
}

void Kopete::ChatSession::addContact( const Kopete::Contact *c, const Kopete::OnlineStatus &initialStatus, bool suppress )
{
	if( !d->contactStatus.contains(c) )
		d->contactStatus[ c ] = initialStatus;
	addContact( c, suppress );
}

void Kopete::ChatSession::addContact( const Kopete::Contact *c, bool suppress )
{
	//kDebug( 14010 ) ;
	if ( d->contacts.contains( (Kopete::Contact*)c ) )
	{
		kDebug( 14010 ) << "Contact already exists";
//		emit contactAdded( c, suppress );
	}
	else
	{
		if ( d->contacts.count() == 1 && d->isEmpty )
		{
			kDebug( 14010 ) << " FUCKER ZONE ";
			/* We have only 1 contact before, so the status of the
			   message manager was given from that contact status */
			Kopete::Contact *old = d->contacts.first();
			d->contacts.removeAll( old );
			d->contacts.append( (Kopete::Contact*)c );

			disconnect( old, SIGNAL(onlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)),
				this, SLOT(slotOnlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)) );
			disconnect( old, SIGNAL(statusMessageChanged(Kopete::Contact*)), this, SIGNAL(statusMessageChanged(Kopete::Contact*)) );

			if ( old->metaContact() )
			{
				disconnect( old->metaContact(), SIGNAL(displayNameChanged(QString,QString)), this, SLOT(slotUpdateDisplayName()) );
				disconnect( old->metaContact(), SIGNAL(photoChanged()), this, SIGNAL(photoChanged()) );
			}
			else
				disconnect( old, SIGNAL(displayNameChanged(QString,QString)), this, SLOT(slotUpdateDisplayName()) );

			disconnect( old, SIGNAL(displayNameChanged(QString,QString)), this, SLOT(slotDisplayNameChanged(QString,QString)) );

			emit contactAdded( c, suppress );
			emit contactRemoved( old, QString() );
		}
		else
		{
			d->contacts.append( (Kopete::Contact*)c );
			emit contactAdded( c, suppress );
		}

		connect( c, SIGNAL(onlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)),
			this, SLOT(slotOnlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)) );
		connect( c, SIGNAL(statusMessageChanged(Kopete::Contact*)), this, SIGNAL(statusMessageChanged(Kopete::Contact*)) );

		if ( c->metaContact() )
		{
			connect( c->metaContact(), SIGNAL(displayNameChanged(QString,QString)), this, SLOT(slotUpdateDisplayName()) );
			connect( c->metaContact(), SIGNAL(photoChanged()), this, SIGNAL(photoChanged()) );
		}
		else
			connect( c, SIGNAL(displayNameChanged(QString,QString)), this, SLOT(slotUpdateDisplayName()) );
		connect( c, SIGNAL(contactDestroyed(Kopete::Contact*)), this, SLOT(slotContactDestroyed(Kopete::Contact*)) );
		connect( c, SIGNAL(displayNameChanged(QString,QString)), this, SLOT(slotDisplayNameChanged(QString,QString)) );

		slotUpdateDisplayName();
	}
	d->isEmpty = false;
}

void Kopete::ChatSession::removeContact( const Kopete::Contact *c, const QString& reason, Qt::TextFormat format, bool suppressNotification )
{
	kDebug( 14010 ) ;
	if ( !c || !d->contacts.contains( (Kopete::Contact*)c ) )
		return;

	if ( d->contacts.count() == 1 )
	{
		kDebug( 14010 ) << "Contact not removed. Keep always one contact";
		d->isEmpty = true;
	}
	else
	{
		d->contacts.removeAll( (Kopete::Contact*)c );

		disconnect( c, SIGNAL(onlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)),
			this, SLOT(slotOnlineStatusChanged(Kopete::Contact*,Kopete::OnlineStatus,Kopete::OnlineStatus)) );

		if ( c->metaContact() )
		{
			disconnect( c->metaContact(), SIGNAL(displayNameChanged(QString,QString)), this, SLOT(slotUpdateDisplayName()) );
			disconnect( c->metaContact(), SIGNAL(photoChanged()), this, SIGNAL(photoChanged()) );
		}
		else
			disconnect( c, SIGNAL(displayNameChanged(QString,QString)), this, SLOT(slotUpdateDisplayName()) );
		disconnect( c, SIGNAL(contactDestroyed(Kopete::Contact*)), this, SLOT(slotContactDestroyed(Kopete::Contact*)) );

		disconnect( c, SIGNAL(displayNameChanged(QString,QString)), this, SLOT(slotDisplayNameChanged(QString,QString)) );

		slotUpdateDisplayName();
	}

	d->contactStatus.remove( c );

	emit contactRemoved( c, reason, format, suppressNotification );
}

void Kopete::ChatSession::receivedTypingMsg( const Kopete::Contact *c, bool t )
{
	emit remoteTyping( c, t );

	if ( ( account()->isAway() && ! Kopete::BehaviorSettings::self()->enableEventsWhileAway() ) || account()->isBusy() )
		return;

	QWidget * viewWidget = dynamic_cast<QWidget*>(view(false));
	bool isActiveWindow = view(false) && ( viewWidget && viewWidget->isActiveWindow() );

	// We aren't interested in notification from current window
	// or 'user stopped typing' notifications
	if ( isActiveWindow || !t )
	{
		return;
	}

	// If there is a notification in d->typingNotifications, then we should show it and quit
	Kopete::ActiveNotifications::iterator notifyIt =
		d->typingNotifications.find( c->account()->accountLabel() + c->contactId() );
	if (notifyIt != d->typingNotifications.end())
	{
		( *notifyIt )->showNotification();
		return;
	}

	KNotification *notification = new KNotification( "user_is_typing_message", viewWidget );
	const QString msgBody = i18n( "User <i>%1</i> is typing a message", c->displayName() );
	notification->setText( msgBody );
	notification->setPixmap( QPixmap::fromImage( c->metaContact()->picture().image() ) );
	notification->setActions( QStringList( i18nc("@action", "Chat") ) );

	new Kopete::ActiveNotification( notification,
							c->account()->accountLabel() + c->contactId(),
							d->typingNotifications,
							"",
							msgBody );

	Kopete::MetaContact *mc = c->metaContact();
	if ( mc )
	{
		notification->addContext( qMakePair( QString::fromLatin1("contact"), mc->metaContactId().toString() ) );
		foreach( Kopete::Group *g , mc->groups() )
		{
			notification->addContext( qMakePair( QString::fromLatin1("group") , QString::number( g->groupId() ) ) );
		}
	}
	connect( notification, SIGNAL(activated(uint)) , c, SLOT(execute()) );
	// User don't need this notification when view is activate
	connect( this, SIGNAL(viewActivated(KopeteView*)), notification, SLOT(close()) );

	notification->sendEvent();
}


void Kopete::ChatSession::receivedTypingMsg( const QString &contactId, bool t )
{
	int i;

	// FIXME: this needs better design. We can't iterate through List to find out who got what ID
	// hash will be better for that, right ?
	for ( i=0; i != d->contacts.size(); i++ )
	{
		if ( (d->contacts[i])->contactId() == contactId )
		{
			receivedTypingMsg( d->contacts[i], t );
			return;
		}
	}
}

void Kopete::ChatSession::typing( bool t )
{
	emit myselfTyping( t );
}

void Kopete::ChatSession::receivedEventNotification( const QString& notificationText)
{
	emit eventNotification( notificationText );
}

void Kopete::ChatSession::receivedMessageState( uint messageId, Kopete::Message::MessageState state )
{
	emit messageStateChanged( messageId, state );
}

void Kopete::ChatSession::setCanBeDeleted ( bool b )
{
	d->mCanBeDeleted = b;
	if (d->refcount < (b?1:0) && !d->view )
		deleteLater();
}

void Kopete::ChatSession::ref ()
{
	d->refcount++;
}
void Kopete::ChatSession::deref ()
{
	d->refcount--;
	if ( d->refcount < 1 && d->mCanBeDeleted && !d->view )
		deleteLater();
}

KopeteView* Kopete::ChatSession::view( bool canCreate, const QString &requestedPlugin )
{
	if ( !d->view && canCreate )
	{
		d->view = Kopete::ChatSessionManager::self()->createView( this, requestedPlugin );
		if ( d->view )
		{
			connect( d->view->mainWidget(), SIGNAL(activated(KopeteView*)), this, SIGNAL(viewActivated(KopeteView*)) );
			connect( d->view->mainWidget(), SIGNAL(closing(KopeteView*)), this, SLOT(slotViewDestroyed()) );
		}
		else
		{
			KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Error,
				i18n( "<qt>An error has occurred while creating a new chat window. The chat window has not been created.</qt>" ),
				i18n( "Error While Creating Chat Window" ) );
		}
	}
	return d->view;
}

void Kopete::ChatSession::slotViewDestroyed()
{
	d->view = 0L;
	if ( d->mCanBeDeleted && d->refcount < 1)
		deleteLater();
}

Kopete::Account *Kopete::ChatSession::account() const
{
	if ( !myself() )
		return 0;

	return myself()->account();
}

void Kopete::ChatSession::slotContactDestroyed( Kopete::Contact *contact )
{
	if( !contact || !d->contacts.contains( contact ) )
		return;

	//This is a workaround to prevent crash if the contact get deleted.
	// in the best case, we should ask the protocol to recreate a temporary contact.
	// (remember: the contact may be deleted when the users removes it from the contact list, or when closing kopete )
	d->contacts.removeAll( contact );
	emit contactRemoved( contact, QString() );

	if ( d->contacts.isEmpty() )
		deleteLater();
}

void Kopete::ChatSession::slotMyselfDestroyed( Kopete::Contact *contact )
{
	Q_UNUSED(contact);
	d->mUser = 0;
	deleteLater();
}

bool Kopete::ChatSession::mayInvite() const
{
	return d->mayInvite;
}

void Kopete::ChatSession::inviteContact(const QString& )
{
	//default implementation do nothing
}

void Kopete::ChatSession::setMayInvite( bool b )
{
	d->mayInvite=b;
}

void Kopete::ChatSession::raiseView()
{
	KopeteView *v=view(true, Kopete::BehaviorSettings::self()->viewPlugin() );
	if(v)
		v->raise(true);
}

Kopete::ChatSession::Form Kopete::ChatSession::form() const
{
	return d->form;
}
bool Kopete::ChatSession::warnGroupChat() const
{
	return d->warnGroupChat;
}

void Kopete::ChatSession::setWarnGroupChat( bool b )
{
	d->warnGroupChat=b;
}

void Kopete::ChatSession::slotDisplayNameChanged(const QString &oldName, const QString &)
{
	Kopete::Contact *c = static_cast<Kopete::Contact *>(sender());
	emit nickNameChanged(c, oldName);
}

#include "kopetechatsession.moc"



// vim: set noet ts=4 sts=4 sw=4:

