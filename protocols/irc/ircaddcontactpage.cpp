#include <qlayout.h>
#include <qlineedit.h>
#include <kdebug.h>
#include <qcheckbox.h>

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
	QObject::connect(ircdata->chkConnectNow, SIGNAL(clicked()), this, SLOT(connectNowClicked()));
}
IRCAddContactPage::~IRCAddContactPage()
{
}
/** No descriptions */
void IRCAddContactPage::slotFinish()
{
	plugin->addContact(ircdata->ircServer->text(), ircdata->addID->text(), ircdata->chkConnectNow->isChecked(), ircdata->chkJoinNow->isChecked());
}

void IRCAddContactPage::connectNowClicked()
{
	if (ircdata->chkConnectNow->isChecked() == true)
	{
		ircdata->chkJoinNow->setEnabled(true);
	} else {
		ircdata->chkJoinNow->setEnabled(false);
		ircdata->chkJoinNow->setChecked(false);
	}
}
