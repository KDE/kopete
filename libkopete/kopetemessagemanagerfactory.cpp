
#include "kopetemessagemanagerfactory.h"

KopeteMessageManagerFactory::KopeteMessageManagerFactory()
{
}

KopeteMessageManagerFactory::~KopeteMessageManagerFactory()
{
}

KopeteMessageManager *KopeteMessageManagerFactory::create( const KopeteContactList &contacts )
{

	KopeteMessageManager *session = new KopeteMessageManager ( contacts );
	(mSessionList).append(session);
	return (session);

}