#include "smsadd.h"
#include "smsaddcontactpage.h"
#include "smsprotocol.h"
#include "kopeteaccount.h"

#include <qlayout.h>
#include <qlineedit.h>

#include <klocale.h>
#include <kmessagebox.h>


SMSAddContactPage::SMSAddContactPage(QWidget *parent, const char *name )
				  : AddContactPage(parent,name)
{
	(new QVBoxLayout(this))->setAutoAdd(true);
	smsdata = new smsAddUI(this);
}

SMSAddContactPage::~SMSAddContactPage()
{

}

bool SMSAddContactPage::apply(KopeteAccount* a, KopeteMetaContact* m)
{
	if ( validateData() )
	{
		QString nr = smsdata->addNr->text();
		QString name = smsdata->addName->text();

		return a->addContact(nr, name, m);
	}
	
	return false;
}


bool SMSAddContactPage::validateData()
{
	return true;
}

#include "smsaddcontactpage.moc"
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

