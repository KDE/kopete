/*
    historypreferences.cpp

    Copyright (c) 2003 by Olivier Goffart        <ogoffart@tiscalinet.be>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

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
#include "historyprefsui.h"
#include <qcheckbox.h>

class HistoryPreferences;

typedef KGenericFactory<HistoryPreferences> HistoryConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_history, HistoryConfigFactory( "kcm_kopete_history" ) )

class HistoryPreferences : public KCAutoConfigModule
{
public:
	HistoryPreferences( QWidget *parent = 0, const char * = 0, const QStringList &args = QStringList() ) : KCAutoConfigModule( HistoryConfigFactory::instance(), parent, args )
	{
		HistoryPrefsUI *p = new HistoryPrefsUI( this );
		p->Auto_chatwindow->setChecked(false);
		setMainWidget( p , "History Plugin");
		
	}
};

