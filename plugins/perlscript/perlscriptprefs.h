/*
    perlplugin.cpp

    Kopete Perl Scriping plugin

    Copyright (c) 2003 by Jason Keirstead   <jason@keirstead.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef PERLSCRIPTPREFERENCES_H
#define PERLSCRIPTPREFERENCES_H

#include <qstringlist.h>
#include "configmodule.h"

class PerlScriptPrefsUI;
class QListViewItem;

class PerlScriptPreferences : public ConfigModule
{
Q_OBJECT
public:
	PerlScriptPreferences(const QString &pixmap,QObject *parent=0);
	~PerlScriptPreferences();
	virtual void save();
	virtual void reopen();
	
signals:
	void saved();
	void scriptAdded( const QString &scriptPath, const QString &scriptName, const QString &scriptDesc );
	void scriptRemoved( const QString &scriptPath );
	void scriptModified( const QString &scriptPath );

private:
	PerlScriptPrefsUI *preferencesDialog;
	int scriptsLoaded;
	QStringList m_scripts;

private slots:
	void slotAddScript( const QString &scriptPath, const QString &scriptName, const QString &scriptDesc, bool init = false );
	void slotNewScript();
	void slotSaveScript();
	void slotRemoveScript();
	void slotSelectionChanged( QListViewItem *selectedScript ); 
};

#endif
