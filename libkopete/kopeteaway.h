#ifndef KOPETEAWAY_HI
#define KOPETEAWAY_HI

#include <qstring.h>
#include <kconfig.h>

class KopeteAway
{
friend class KopeteAwayDialog;
	
	public:
	KopeteAway();
	~KopeteAway();
	
	QString message();
	void show();

	private:
	QString mAwayMessage;
	KConfig *config;
};

#endif
