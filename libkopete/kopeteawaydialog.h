#ifndef KOPETEAWAYDIALOG_HI
#define KOPETEAWAYDIALOG_HI

class QString;

#include "kopeteawaydialogbase.h"

class KopeteAwayDialog : public KopeteAwayBase
{
	Q_OBJECT

	public:
	KopeteAwayDialog();
	
	private slots:
		void slotOkayClicked();
		void slotCancelClicked();
};

#endif
