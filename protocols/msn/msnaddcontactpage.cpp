#include <qlayout.h>
#include <qlineedit.h>

#include <klocale.h>

#include "msnadd.h"
#include "msnaddcontactpage.h"
#include "msnprotocol.h"

MSNAddContactPage::MSNAddContactPage(MSNProtocol *owner, QWidget *parent, const char *name )
				  : AddContactPage(parent,name)
{
	(new QVBoxLayout(this))->setAutoAdd(true);
	if ( owner->isConnected() )
	{
			msndata = new msnAddUI(this);
			/*			
			msndata->cmbGroup->insertStringList(owner->getGroups());
			msndata->cmbGroup->setCurrentItem(0);
			*/
			plugin = owner;
			canadd = true;
			
	}
	else
	{
			noaddMsg1 = new QLabel(i18n("You need to be connected to be able to add contacts."), this);
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
			KMessageBox::sorry(this, i18n("<qt>I'm sorry, you need to have a buddy listed under a group.</qt>"), i18n("You Must Select a Group"));
			return;
		}
		*/
		QString userid = msndata->addID->text();
		plugin->addContact( userid );
	}
	else
	{
		return;
	}
}

bool MSNAddContactPage::validateData()
{
	// PLEASE DO SOME VALIDATING HERE FOR MSN ID'S!!!
	return true;
}

#include "msnaddcontactpage.moc"
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

