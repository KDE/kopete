#include "smsaccount.h"
#include "smsprotocol.h"
#include "smscontact.h"

SMSAccount::SMSAccount( SMSProtocol *parent, const QString &accountID, const char *name )
	: KopeteAccount( parent, accountID, name )
{
	m_myself = new SMSContact( this, accountID, accountID, 0L );
}

SMSAccount::~SMSAccount()
{

}

void SMSAccount::setAway( bool /*away*/, const QString &)
{

}

void SMSAccount::connect()
{
//	m_mySelf->setOnlineStatus( SMSOnline );

	// FIXME: Set all contacts to SMSUnknown here
}

void SMSAccount::disconnect()
{
//	m_mySelf->setOnlineStatus( SMSOffline );

	// FIXME: Set all contacts to SMSOffline here
}

KopeteContact* SMSAccount::myself() const
{
	return m_myself;
}

bool SMSAccount::addContactToMetaContact( const QString &contactId, const QString &displayName,
	KopeteMetaContact * parentContact )
{
	if (new SMSContact(this, contactId, displayName, parentContact))
		return true;
	else
		return false;
}

#include "smsaccount.moc"
