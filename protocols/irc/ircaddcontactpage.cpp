#include <qlayout.h>
#include <qlineedit.h>
#include <kdebug.h>

#include "ircaddcontactpage.h"
#include <ircadd.h>
#include <ircprotocol.h>

#include <kglobal.h>
#include <kconfig.h>

IRCAddContactPage::IRCAddContactPage(IRCProtocol *owner, QWidget *parent, const char *name )
				  : AddContactPage(parent,name)
{
	(new QVBoxLayout(this))->setAutoAdd(true);
	ircdata = new ircAddUI(this);
	plugin = owner;	
}
IRCAddContactPage::~IRCAddContactPage()
{
}
/** No descriptions */
void IRCAddContactPage::slotFinish()
{
/*
	QString nick = msndata->addNick->text();
	QString userid = msndata->addID->text();
	plugin->engine->contactAdd(userid);
*/
}
