#include <qlayout.h>
#include <qlineedit.h>
#include <kdebug.h>
#include "msnaddcontactpage.h"
#include <msnadd.h>
#include <msnprotocol.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>

MSNAddContactPage::MSNAddContactPage(MSNProtocol *owner, QWidget *parent, const char *name )
				  : AddContactPage(parent,name)
{
	(new QVBoxLayout(this))->setAutoAdd(true);
	if ( owner->isConnected() )
	{
			msndata = new msnAddUI(this);
			plugin = owner;
			canadd = true;	
	}
	else
	{
			noaddMsg1 = new QLabel(i18n("Sorry, you need to be connected to be able to add contacts."), this);
			noaddMsg2 = new QLabel(i18n("Connect to the MSN network and try again."), this);
			canadd = false;
	}
	
}
MSNAddContactPage::~MSNAddContactPage()
{
}
/** No descriptions */
void MSNAddContactPage::slotFinish()
{
	if ( canadd )
	{
		QString nick = msndata->addNick->text();
		QString userid = msndata->addID->text();
		plugin->engine->contactAdd(userid);
	}
	else
	{
		return;
	}
}
