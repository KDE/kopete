#include "kopeteawaydialog.h"
#include "kopeteawaydialog.moc"

#include <qstring.h>
#include <qdialog.h>
#include <qpushbutton.h>
//#include <qlabel.h>
#include <qmultilineedit.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kcombobox.h>

#include "kopete.h"
#include "kopeteaway.h"

KopeteAwayDialog::KopeteAwayDialog()
{
	
	
	connect ( cmdCancel, SIGNAL(clicked()), this, SLOT(slotCancelClicked()) );
	connect ( cmdOkay, SIGNAL(clicked()), this, SLOT(slotOkayClicked()) );
	
	/* Get the list of away messages */
	QStringList titles = KopeteAway::getInstance()->getTitles(); // Get the titles
	for(QStringList::iterator i = titles.begin(); i != titles.end(); i++){
		cmbHistory->insertItem((*i)); // Should be a QString item....
	}
}

void KopeteAwayDialog::slotCancelClicked()
{
	close();
}

void KopeteAwayDialog::slotOkayClicked()
{
	/* Set the global away message */
	KopeteAway::getInstance()->mAwayMessage = KopeteAway::getInstance()->getMessage(cmbHistory->currentText());

	close();
}
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

