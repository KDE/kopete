
#include "kopetemessagemanager.h"

KopeteMessageManager::KopeteMessageManager( const KopeteContactList &contacts,
		QObject *parent = 0, const char *name = 0 ) : QObject( parent, name)
{


}

KopeteMessageManager* KopeteMessageManager::createSession( const KopeteContactList &contacts )
{
	
 return (new KopeteMessageManager ( contacts ) );

}

KopeteMessageManager::~KopeteMessageManager()
{

}

void KopeteMessageManager::appendMessage( const KopeteMessage *msg )
{

}
void KopeteMessageManager::addContact( const KopeteContact *c )
{
}
void KopeteMessageManager::removeContact( const KopeteContact *c )
{
}

