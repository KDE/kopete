#include "smsservice.h"

SMSService::SMSService(KopeteAccount* account)
	: QObject(), m_account(account)
{
}

SMSService::~SMSService()
{

}

void SMSService::setAccount(KopeteAccount* account)
{
	if(!m_account)
		m_account = account;
	if(account)
		savePreferences();
}

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

