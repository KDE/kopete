/*
    kopetepluginconfig.h - Configure the Kopete plugins

    Copyright (c) 2003      by Martijn Klingens      <klingens@kde.org>

    Kopete    (c) 2001-2003 by the Kopete developers <kopete-devel@kde.org>

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

#include <kdialogbase.h>

class KopetePluginConfigPrivate;

/**
 * Plugin selector. See KPluginSelector in kdelibs for documentation.
 *
 * @author Martijn Klingens <klingens@kde.org>
 */
class KopetePluginConfig : public KDialogBase
{
	Q_OBJECT

public:
	KopetePluginConfig( QWidget *parent, const char *name = 0L );
	~KopetePluginConfig();
	void apply();

public slots:
	void setChanged( bool c );

	virtual void slotDefault();
	virtual void slotUser1();
	virtual void slotApply();
	virtual void slotOk();
	virtual void slotHelp();
	virtual void show();

private:
	KopetePluginConfigPrivate *d;
};

#endif // KOPETEPLUGINCONFIG_H

// vim: set noet ts=4 sts=4 sw=4:

