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
	static void setGlobalAway(bool status);
	static bool globalAway();

	private:
	KopeteAway();
	static KopeteAway *instance;
	QString mAwayMessage;
	bool mGlobalAway;
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

