// -*- c++ -*-
/***************************************************************************
 *									   *
 *   Copyright: SuSE Linux AG, Nuernberg				   *
 *									   *
 *   Author: Arvin Schnell <arvin@suse.de>				   *
 *									   *
 ***************************************************************************/

/***************************************************************************
 *									   *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or	   *
 *   (at your option) any later version.				   *
 *									   *
 ***************************************************************************/


#ifndef KINTERNETIFACE_H
#define KINTERNETIFACE_H


#include <dcopobject.h>

class KInternetIface : public DCOPObject
{
    K_DCOP

public:

    KInternetIface (const QCString& name) : DCOPObject (name) { }

k_dcop:

    // query function for susewatcher
    bool isOnline () {
#ifndef NDEBUG
	fprintf (stderr, "%s\n", __PRETTY_FUNCTION__);
#endif
	return kinternet && kinternet->get_status () == KInternet::CONNECTED;
    }

};


#endif
