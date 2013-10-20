/*
    kopetepluginconfig.h - Configure the Kopete plugins

    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>
    Copyright (c) 2006      by Michaël Larouche      <larouche@kde.org>
    Copyright (c) 2007      by Will Stephenson       <wstephenson@kde.org>

    Kopete    (c) 2001-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEPLUGINCONFIG_H
#define KOPETEPLUGINCONFIG_H

#include <kcmodule.h>

class KPluginSelector;

/**
 * Plugin selector. See KPluginSelector in kdelibs for documentation.
 *
 * @author Martijn Klingens <klingens@kde.org>
 */
class KopetePluginConfig : public KCModule
{
	Q_OBJECT

public:
	KopetePluginConfig( QWidget *parent, const QVariantList &args  );
	~KopetePluginConfig();

public slots:
	virtual void load();
	virtual void save();

	virtual void defaults();
    void reparseConfiguration(const QByteArray&conf);
private:
	KPluginSelector *m_pluginSelector;
};

#endif // KOPETEPLUGINCONFIG_H

// vim: set noet ts=4 sts=4 sw=4:

