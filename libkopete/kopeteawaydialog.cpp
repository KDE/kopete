#include "kopeteawaydialog.h"
#include "kopeteawaydialog.moc"

#include <qstring.h>
#include <qdialog.h>
#include <qpushbutton.h>
//#include <qlabel.h>
#include <qmultilineedit.h>
#include <kglobal.h>
#include <kconfig.h>

#include <kopete.h>

KopeteAwayDialog::KopeteAwayDialog()
{
	connect ( cmdCancel, SIGNAL(clicked()), this, SLOT(slotCancelClicked()) );
	connect ( cmdOkay, SIGNAL(clicked()), this, SLOT(slotOkayClicked()) );
}

void KopeteAwayDialog::slotCancelClicked()
{
	close();
}

void KopeteAwayDialog::slotOkayClicked()
{
	kopeteapp->away()->mAwayMessage = mleMessage->text();
	close();
        kopeteapp->away()->config->setGroup("");
        kopeteapp->away()->config->writeEntry ( "AwayMessage", kopeteapp->away()->mAwayMessage );
        kopeteapp->away()->config->sync();
				
}
