#include <qlayout.h>
#include <qlineedit.h>
#include <kdebug.h>
#include "msnaddcontactpage.h"
#include <msnadd.h>
#include <msnprotocol.h>
#include <kglobal.h>
#include <kconfig.h>

MSNAddContactPage::MSNAddContactPage(MSNProtocol *owner, QWidget *parent, const char *name )
				  : AddContactPage(parent,name)
{
	(new QVBoxLayout(this))->setAutoAdd(true);
	msndata = new msnAddUI(this);
	plugin = owner;	
}
MSNAddContactPage::~MSNAddContactPage()
{
}
/** No descriptions */
void MSNAddContactPage::slotFinish()
{
	QString nick = msndata->addNick->text();
	QString userid = msndata->addID->text();
	plugin->engine->slotAddUser(userid,nick);
}
