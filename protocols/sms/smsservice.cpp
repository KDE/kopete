#include "smsservice.h"

SMSService::SMSService(SMSContact* contact)
	: QObject()
{
	m_contact = contact;
}

SMSService::~SMSService()
{

}

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

