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
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

