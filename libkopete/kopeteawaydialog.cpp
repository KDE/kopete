#include "kopeteawaydialog.h"
#include "kopeteawaydialog.moc"

#include <qstring.h>
#include <qdialog.h>
#include <qpushbutton.h>
//#include <qlabel.h>
#include <qmultilineedit.h>
#include <kglobal.h>
#include <kconfig.h>
#include <qcombobox.h>

#include "kopete.h"
#include "kopeteaway.h"

KopeteAwayDialog::KopeteAwayDialog()
{
	connect ( cmdCancel, SIGNAL(clicked()), this, SLOT(slotCancelClicked()) );
	connect ( cmdOkay, SIGNAL(clicked()), this, SLOT(slotOkayClicked()) );
	connect ( cmbHistory, SIGNAL(textChanged()), this, SLOT(slotComboChanged()) );

	config = KGlobal::config();
        config->setGroup("AwayMessage");
        cmbHistory->insertItem(config->readEntry("AwayMessage0", "I'm away"), 0);
        cmbHistory->insertItem(config->readEntry("AwayMessage1", "Will be back soon ..."), 1);
        cmbHistory->insertItem(config->readEntry("AwayMessage2", "Watching TV"), 2);
        cmbHistory->insertItem(config->readEntry("AwayMessage3", "Eating"), 3);
        cmbHistory->insertItem(config->readEntry("AwayMessage4", "Sleeping"), 4);
        cmbHistory->insertItem(config->readEntry("AwayMessage5", "Don't disturb me"), 5);
	cmbHistory->setCurrentItem(config->readNumEntry("AwayNumber",0));
}

void KopeteAwayDialog::slotCancelClicked()
{
	close();
}

void KopeteAwayDialog::slotOkayClicked()
{
	KopeteAway::getInstance()->mAwayMessage = cmbHistory->currentText();
        config->setGroup("AwayMessage");
        config->writeEntry ( QString("AwayMessage%1").arg(cmbHistory->currentItem()), cmbHistory->currentText() );
	config->writeEntry ( "AwayNumber", cmbHistory->currentItem() );
        config->sync();
	close();
}
