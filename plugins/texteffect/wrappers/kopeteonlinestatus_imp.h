/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KOPETE_ONLINE_STATUS_IMP_H
#define _KOPETE_ONLINE_STATUS_IMP_H

#include <kopeteonlinestatus.h>
#include "bindingobject.h"

class Status : public BindingObject
{
	Q_OBJECT

	public:
		Status( const KopeteOnlineStatus &s, QObject *parent=0, const char *name=0 );

	private:
		KopeteOnlineStatus *status;
};

#endif

