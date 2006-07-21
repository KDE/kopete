/*
    kopetepluginconfig.h - Configure the Kopete plugins

    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2001-2006 by the Kopete developers <kopete-devel@kde.org>

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

#include <kdialog.h>

class KopetePluginConfigPrivate;

/**
 * Plugin selector. See KPluginSelector in kdelibs for documentation.
 *
 * @author Martijn Klingens <klingens@kde.org>
 */
class KopetePluginConfig : public KDialog
{
	Q_OBJECT

public:
	KopetePluginConfig( QWidget *parent );
	~KopetePluginConfig();

public slots:
	void slotDefault();
	void slotReset();
	void slotApply();
	void slotHelp();

	virtual void accept();

private slots:
	void setChanged( bool c );

private:
	KopetePluginConfigPrivate *d;
};

#endif // KOPETEPLUGINCONFIG_H

// vim: set noet ts=4 sts=4 sw=4:

