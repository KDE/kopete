#include <qcheckbox.h>
#include <qlayout.h>
#include <qlineedit.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qcombobox.h>

#include "ircaddcontactpage.h"
#include "ircadd.h"
#include "ircprotocol.h"
#include "kopete.h"
#include "kopetecontactlist.h"

IRCAddContactPage::IRCAddContactPage(IRCProtocol *owner, QWidget *parent, const char *name )
				  : AddContactPage(parent,name)
{
	(new QVBoxLayout(this))->setAutoAdd(true);
	ircdata = new ircAddUI(this);
	plugin = owner;
	QObject::connect(ircdata->chkConnectNow, SIGNAL(clicked()), this, SLOT(connectNowClicked()));

	KGlobal::config()->setGroup("IRC");
	QString server = KGlobal::config()->readEntry("Server", "");
	ircdata->ircServer->setText(server);
}
IRCAddContactPage::~IRCAddContactPage()
{
}

void IRCAddContactPage::slotFinish(KopeteMetaContact *m)
{
	QString server = ircdata->ircServer->text();
	QString name = ircdata->addID->text();
	plugin->addContact(server, name, ircdata->chkConnectNow->isChecked(), ircdata->chkJoinNow->isChecked(),m);
}

bool IRCAddContactPage::validateData()
{
	QString server = ircdata->ircServer->text();
	if (server.isEmpty() == true)
	{
		KMessageBox::sorry(this, i18n("<qt>You need to specify a server to connect to.</qt>"), i18n("You Must Specify a Server"));
		return false;
	}
	QString name = ircdata->addID->text();
	if (name.isEmpty() == true)
	{
		KMessageBox::sorry(this, i18n("<qt>You need to specify a channel to join, or query to open.</qt>"), i18n("You Must Specify a Channel"));
		return false;
	}
	/*if(name.contains('@'))
	{
		KMessageBox::sorry(this, i18n("<qt>Bad charactere (@) in channel name</qt>"), i18n("You Must Specify a Channel"));
		return false;
	} */
  return true;
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
#include "ircaddcontactpage.moc"
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:


