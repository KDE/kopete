
#include "kopetemessagemanager.h"
#include "kopetechatwindow.h"
#include "kopeteevent.h"
#include "kopete.h"

#include "messagelog.h"
#include <kdebug.h>

KopeteMessageManager::KopeteMessageManager( const KopeteContact *user , KopeteContactList others,
		QString logFile = QString::null , QObject *parent = 0, const char *name = 0 ) : QObject( parent, name)
{

	mContactList = others;
	mUser = user;
	mChatWindow = 0L;
	mUnreadMessageEvent = 0L;
	if ( kopeteapp->appearance()->useQueue() )
	{
		mReadMode = Queued;
	}
	else
	{
		mReadMode = Popup;
	}
	
	if (!logFile.isEmpty())
	{
		QString logFileName = "kopete/" + logFile;
		mLogger = new KopeteMessageLog(logFileName, this);
	}
	else
	{
		mLogger = 0L;
	}

}

KopeteMessageManager::~KopeteMessageManager()
{
	emit dying(this);
}

void KopeteMessageManager::setReadMode( int mode )
{

	if ( (mode == Queued) || (mode == Popup) )
	{
		mReadMode = mode;
	}
	else
	{
		kdDebug() << "[KopeteMessageManager] ERROR: unknown reading method, setting to default" << endl;
		mReadMode = Queued;
	}
}

void KopeteMessageManager::readMessages()
{
	if ( mChatWindow != 0L )	// We still have our messagebox
	{
		kdDebug() << "[KopeteMessageManager] mChatWindow has already been created" << endl;
		if (mChatWindow->isMinimized() )
			kdDebug() << "[KopeteMessageManager] mChatWindow is minimized" << endl;
		if (mChatWindow->isHidden() )
			kdDebug() << "[KopeteMessageManager] mChatWindow is hidden" << endl;
		mChatWindow->raise();	// make it top window
	}
	else
	{
		/* We create the chat window */
		mChatWindow = new KopeteChatWindow ();
		/* When the window is shown, we have to delete tjis contact event */
		kdDebug() << "[KopeteMessageManager] Connecting message box shown() to event killer" << endl;
		connect ( mChatWindow, SIGNAL(shown()), this, SLOT(cancelUnreadMessageEvent()) );
		connect ( mChatWindow, SIGNAL(sendMessage(const QString &)), this, SLOT(messageSentFromWindow(const QString &)) );
		connect ( mChatWindow, SIGNAL(destroyed()), this, SLOT(chatWindowClosing()) );
	}
	
	KopeteMessage *tmp;
	while ( tmp = mMessageQueue.take() )
	{
			mChatWindow->messageReceived( *(tmp) );
	}
	mChatWindow->show();	// show message window again
	
}

void KopeteMessageManager::messageSentFromWindow(const QString &message)
{
	QString body = message;
	KopeteMessage tmpmessage(mUser->userID(), (mContactList.first())->userID(), body, KopeteMessage::Outbound);
	emit messageSent (tmpmessage);
}

void KopeteMessageManager::chatWindowClosing()
{
	mChatWindow = 0L;
}

void KopeteMessageManager::cancelUnreadMessageEvent()
{
	if (mUnreadMessageEvent == 0L)
	{
		kdDebug() << "[KopeteMessageManager] No event to delete" << endl;
	}
	else
	{
		kdDebug() << "[KopeteMessageManager] cancelUnreadMessageEvent Deleting Event" << endl;
		delete mUnreadMessageEvent;
		mUnreadMessageEvent = 0L;
	}		
}


void KopeteMessageManager::appendMessage( const KopeteMessage &msg )
{
	mMessageQueue.append( &msg);
	if( mLogger )
	{
		mLogger->append( msg );
	}

	/* We dont need an event if it already exits or if we are in popup mode */
	if ( (mUnreadMessageEvent == 0L) && ( mReadMode != Popup) && (msg.direction() == KopeteMessage::Inbound) )
	{
		mUnreadMessageEvent = new KopeteEvent( msg.from(), "newmsg", this, SLOT(readMessages()));
		kopeteapp->notifyEvent( mUnreadMessageEvent );
	}

	if (mReadMode == Popup)
	{
		readMessages();
	}
}
void KopeteMessageManager::addContact( const KopeteContact *c )
{
	KopeteContact *tmp;

	for ( tmp = mContactList.first(); tmp; tmp = mContactList.next() )
	{
		if ( tmp == c )
		{
			kdDebug() << "[KopeteMessageManager] Contact already exists" <<endl;
			return;
		}
	}

	kdDebug() << "[KopeteMessageManager] Contact Joined session" <<endl;
	mContactList.append(c);
}

void KopeteMessageManager::removeContact( const KopeteContact *c )
{
	mContactList.take( mContactList.find(c) );
}

