
#ifndef SMSADDCONTACTPAGE_H
#define SMSADDCONTACTPAGE_H

#include <qwidget.h>
#include <addcontactpage.h>
#include <qlabel.h>

class smsAddUI;
class SMSProtocol;

class SMSAddContactPage : public AddContactPage
{
   Q_OBJECT
public:
	SMSAddContactPage(QWidget *parent=0, const char *name=0);
	~SMSAddContactPage();
	smsAddUI *smsdata;
	virtual bool validateData();
	virtual bool apply( KopeteAccount*, KopeteMetaContact* );
};


#endif



/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

