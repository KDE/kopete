/*
    msnpreferences.cpp - MSN Preferences Widget

    Copyright (c) 2002-2003 by Olivier Goffart        <ogoffart @ kde.org>
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <kgenericfactory.h>
#include "kcautoconfigmodule.h"
#include "msnprefs.h"

class MSNPreferences;

typedef KGenericFactory<MSNPreferences> MSNProtocolConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_msn, MSNProtocolConfigFactory( "kcm_kopete_msn" ) )

class MSNPreferences : public KCAutoConfigModule
{
public:
	MSNPreferences( QWidget *parent = 0, const char * = 0, const QStringList &args = QStringList() ) : KCAutoConfigModule( MSNProtocolConfigFactory::instance(), parent, args )
	{
		setMainWidget( new msnPrefsUI( this ) , "MSN");
	}
};
