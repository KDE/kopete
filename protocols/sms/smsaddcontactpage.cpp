#include "smsadd.h"
#include "smsaddcontactpage.h"
#include "smsprotocol.h"

#include <qlayout.h>
#include <qlineedit.h>

#include <klocale.h>
#include <kmessagebox.h>


SMSAddContactPage::SMSAddContactPage(SMSProtocol *owner, QWidget *parent, const char *name )
				  : AddContactPage(parent,name)
{
	(new QVBoxLayout(this))->setAutoAdd(true);
	smsdata = new smsAddUI(this);
	plugin = owner;
}

SMSAddContactPage::~SMSAddContactPage()
{

}

void SMSAddContactPage::slotFinish(KopeteMetaContact *m)
{
	QString nr = smsdata->addNr->text();
	QString name = smsdata->addName->text();
	
	plugin->addContact( nr, name, m );
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

