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
#include <kopeteaway.h>

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
	awayInstance = KopeteAway::getInstance();
	awayInstance->mAwayMessage = mleMessage->text();
	close();
        awayInstance->config->setGroup("");
        awayInstance->config->writeEntry ( "AwayMessage", awayInstance->mAwayMessage );
        awayInstance->config->sync();
				
}
