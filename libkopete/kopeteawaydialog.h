#ifndef KOPETEAWAYDIALOG_HI
#define KOPETEAWAYDIALOG_HI

class QString;
class KopeteAway;

#include "kopeteawaydialogbase.h"

class KopeteAwayDialog : public KopeteAwayBase
{
	Q_OBJECT

	public:
	KopeteAwayDialog();
	
	private:
	KopeteAway *awayInstance;
	
	private slots:
		void slotOkayClicked();
		void slotCancelClicked();
};

#endif
