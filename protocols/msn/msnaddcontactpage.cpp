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
			/*			
			msndata->cmbGroup->insertStringList(owner->msnService()->getGroups());
			msndata->cmbGroup->setCurrentItem(0);
			*/
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
		/*
		QString currentGroup = msndata->cmbGroup->currentText();
		if (currentGroup.isEmpty() == true)
		{
			KMessageBox::sorry(this, i18n("<qt>I'm sorry, you need to have a buddy listed under a group.</qt>"), i18n("You must select a group"));
			return;
		}
		*/
		QString userid = msndata->addID->text();
		plugin->msnService()->contactAdd(userid);
	}
	else
	{
		return;
	}
}
#include "msnaddcontactpage.moc"
