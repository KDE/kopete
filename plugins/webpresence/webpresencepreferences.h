/*
    webpresencepreferences.h

    Kopete Web Presence plugin

    Copyright (c) 2002 by Will Stephenson <will@stevello.free-online.co.uk>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                    	*
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef WEBPRESENCEPREFERENCES_H
#define WEBPRESENCEPREFERENCES_H

#include "configmodule.h"

class WebPresencePrefsUI;

class WebPresencePreferences : public ConfigModule
{
	Q_OBJECT
	public:
		WebPresencePreferences( const QString &pixmap,QObject *parent=0 );
		virtual ~WebPresencePreferences();
		virtual void save();

		int frequency() const;
		QString url() const;
		bool showAddresses() const;
		bool useImName() const;
		QString userName() const;
		bool useDefaultStyleSheet() const;
		bool justXml() const;
		QString userStyleSheet() const;
		
	signals:
		void saved();
	private:
		WebPresencePrefsUI *m_prefsDialog;
};

#endif
// vim: set noet ts=4 sts=4 sw=4:

