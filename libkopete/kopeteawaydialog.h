#ifndef KOPETEAWAYDIALOG_HI
#define KOPETEAWAYDIALOG_HI

class QString;
class KopeteAway;
class KConfig;

#include "kopeteawaydialogbase.h"

class KopeteAwayDialog : public KopeteAwayBase
{
	Q_OBJECT

	public:
	KopeteAwayDialog();
	
	private:
	KopeteAway *awayInstance;
	KConfig *config;
	
	private slots:
		void slotOkayClicked();
		void slotCancelClicked();
};

#endif
