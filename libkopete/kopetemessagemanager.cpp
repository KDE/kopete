
#include "kopetemessagemanager.h"
#include "kopetechatwindow.h"
#include <kdebug.h>

KopeteMessageManager::KopeteMessageManager( const KopeteContactList &contacts,
		QObject *parent = 0, const char *name = 0 ) : QObject( parent, name)
{


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
		//mChatWindow = new KopeteChatWindow ( this, mUIN, mName, mStatus, mProtocol );
		/* When the window is shown, we have to delete tjis contact event */
        kdDebug() << "[KopeteMessageManager] Connecting message box shown() to event killer" << endl;
		connect ( mChatWindow, SIGNAL(shown()), this, SLOT(slotCancelUnreadMessagesEvent()) );
		connect ( mChatWindow, SIGNAL(destroyed()), this, SLOT(slotMessageBoxClosing()) );
	}
	mChatWindow->show();	// show message window again
	
}

void KopeteMessageManager::appendMessage( const KopeteMessage *msg )
{
	
	mMessageQueue.append(msg);
}
void KopeteMessageManager::addContact( const KopeteContact *c )
{
}
void KopeteMessageManager::removeContact( const KopeteContact *c )
{
}

