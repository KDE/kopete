#ifndef KOPETEAWAY_HI
#define KOPETEAWAY_HI

#include <qstring.h>
#include <kconfig.h>

class KopeteAway
{
friend class KopeteAwayDialog;
	
	public:
	static KopeteAway *getInstance();
	
	static QString message();
	static void show();

	private:
	KopeteAway();
	static KopeteAway *instance;
	QString mAwayMessage;
	KConfig *config;
};

#endif
