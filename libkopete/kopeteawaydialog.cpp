#include "kopeteawaydialog.h"

#include <qdialog.h>
#include <qpushbutton.h>
#include <kcombobox.h>
#include <klineedit.h>

#include "kopeteaway.h"
#include "kopeteidentitymanager.h"

KopeteAwayDialog::KopeteAwayDialog(QWidget *parent, const char *name)
				: KopeteAwayBase(parent, name)
{

		// Connect the buttons to actions
		connect ( cmdCancel, SIGNAL(clicked()),
						this, SLOT(slotCancelClicked()) );
		connect ( cmdOkay, SIGNAL(clicked()),
						this, SLOT(slotOkayClicked()) );

		// Get the KopeteAway instance
		awayInstance = KopeteAway::getInstance();
				
		// Get the list of away messages
		// Insert the string list of titles
		cmbHistory->insertStringList(awayInstance->getTitles());
		
		// Set the text field to ""
		txtOneShot->setText("");
}

KopeteAwayDialog::~KopeteAwayDialog(){}

void KopeteAwayDialog::slotCancelClicked()
{
	close();
}

QString KopeteAwayDialog::getSelectedAwayMessage(){
		QString retMessage = "";
		if(txtOneShot->text() != ""){
				retMessage = txtOneShot->text();
		} else {
				retMessage = awayInstance->getMessage(
								cmbHistory->currentText());
		}
		return retMessage;
}

void KopeteAwayDialog::slotOkayClicked()
{
	// Set the global away message
		awayInstance->mAwayMessage =
			getSelectedAwayMessage();
	
	KopeteIdentityManager::manager()->setAwayAll();
	close();
}



#include "kopeteawaydialog.moc"
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */

// vim: set noet ts=4 sts=4 sw=4:

