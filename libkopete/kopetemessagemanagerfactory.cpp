
#include "kopetemessagemanagerfactory.h"

KopeteMessageManagerFactory::KopeteMessageManagerFactory()
{
}

KopeteMessageManagerFactory::~KopeteMessageManagerFactory()
{
}

KopeteMessageManager *KopeteMessageManagerFactory::create( const KopeteContact *user,KopeteContactList &contacts ,QString logFile = QString::null )
{

	KopeteMessageManager *session = new KopeteMessageManager ( user, contacts , logFile);
	(mSessionList).append(session);
	return (session);

}