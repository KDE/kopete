#ifndef KopeteIface_h
#define KopeteIface_h

#include <dcopobject.h>
#include <qstringlist.h>


/**
 * DCOP interface for kopete
 */
class KopeteIface : virtual public DCOPObject
{
K_DCOP
public:

	KopeteIface();

k_dcop:

	QStringList all_contacts();
	QStringList reachable_contacts();
	QStringList online_contacts();
	QStringList contacts_status();

private:


};

#endif

