
#include "kopetemessagemanager.h"
#include "kopetechatwindow.h"
#include "kopeteevent.h"
#include "messagelog.h"
#include <kdebug.h>

KopeteMessageManager::KopeteMessageManager( const KopeteContact *user , KopeteContactList &others,
		QString logFile = QString::null , QObject *parent = 0, const char *name = 0 ) : QObject( parent, name)
{

	mContactList = others;
	mChatWindow = 0L;
	mUnreadMessageEvent = 0L;
	mReadMode = Queued;

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
		connect ( mChatWindow, SIGNAL(destroyed()), this, SLOT(chatWindowClosing()) );
	}
	
	KopeteMessage *tmp;
	while ( tmp = mMessageQueue.take() )
	{
			mChatWindow->messageReceived( *(tmp) );
	}
	mChatWindow->show();	// show message window again
	
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

	if ( mUnreadMessageEvent == 0L )
	{
		mUnreadMessageEvent = new KopeteEvent( msg.from(), "newmsg", this, SLOT(readMessages()));
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

