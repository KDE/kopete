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
#include "kopetecontactlistview.h"

IRCAddContactPage::IRCAddContactPage(IRCProtocol *owner, QWidget *parent, const char *name )
				  : AddContactPage(parent,name)
{
	(new QVBoxLayout(this))->setAutoAdd(true);
	ircdata = new ircAddUI(this);
	plugin = owner;
	QObject::connect(ircdata->chkConnectNow, SIGNAL(clicked()), this, SLOT(connectNowClicked()));
	ircdata->cmbGroup->insertStringList(kopeteapp->contactList()->groups());
	ircdata->cmbGroup->setCurrentItem(0);
}
IRCAddContactPage::~IRCAddContactPage()
{
}
/** No descriptions */
void IRCAddContactPage::slotFinish()
{
	QString currentGroup = ircdata->cmbGroup->currentText();
	if (currentGroup.isEmpty() == true)
	{
		KMessageBox::sorry(this, i18n("<qt>You need to have a buddy listed under a group. Please create a group first. You may do this by right clicking in the buddy list and selecting \"Add Group...\"</qt>"), i18n("You Must Select a Group"));
		return;
	}
	QString server = ircdata->ircServer->text();
	if (server.isEmpty() == true)
	{
		KMessageBox::sorry(this, i18n("<qt>You need to specify a server to connect to. Please try again. Aborting.</qt>"), i18n("You Must Specify a Server"));
		return;
	}
	QString name = ircdata->addID->text();
	if (name.isEmpty() == true)
	{
		KMessageBox::sorry(this, i18n("<qt>You need to specify a channel to join, or query to open. Please try again. Aborting.</qt>"), i18n("You Must Specify a Channel"));
		return;
	}
	plugin->addContact(currentGroup, server, name, ircdata->chkConnectNow->isChecked(), ircdata->chkJoinNow->isChecked());
}

/** No descriptions */
bool IRCAddContactPage::validateData()
{
	QString currentGroup = ircdata->cmbGroup->currentText();
	if (currentGroup.isEmpty() == true)
	{
		KMessageBox::sorry(this, i18n("<qt>You need to have a buddy listed under a group. Please create a group first. You may do this by right clicking in the buddy list and selecting \"Add Group...\"</qt>"), i18n("You Must Select a Group"));
		return false;
	}
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


