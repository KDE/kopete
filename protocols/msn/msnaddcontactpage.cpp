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
	QString lastname = "Mac-Vicar";
	QString nick = msndata->addNick->text();
	QString firstname = "Duncan";
	QString email = "lala@lala.org";
	//                                              UIN                       Auth   Notify Nick  ?  Firstname Lastname email  unused requestinfo
	//plugin->addContact(icqdata->addUIN->text().toUInt(), false, false, nick, "", firstname, lastname, email, false, false);
}
