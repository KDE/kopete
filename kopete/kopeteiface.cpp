#include "kopeteiface.h"
#include "kopetecontactlist.h"

#include <kapplication.h>
#include <dcopobject.h>
#include <dcopclient.h>
#include <qstringlist.h>


KopeteIface::KopeteIface() : DCOPObject( "KopeteIface" )
{
}

QStringList KopeteIface::all_contacts()
{
	return KopeteContactList::meta_all();
}

QStringList KopeteIface::reachable_contacts()
{
	return KopeteContactList::meta_reachable();
}

QStringList KopeteIface::online_contacts()
{
	return KopeteContactList::meta_online();
}

QStringList KopeteIface::contacts_status()
{
	return KopeteContactList::meta_status();
}
