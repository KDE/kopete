/*
    appearanceconfig.h  -  Kopete Look Feel Config

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2005      by Michaël Larouche       <larouche@kde.org>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef APPEARANCECONFIG_H
#define APPEARANCECONFIG_H

#include <kcmodule.h>

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 * @author Michaël Larouche <larouche@kde.org>
 */
class AppearanceConfig : public KCModule
{
	Q_OBJECT


public:
	AppearanceConfig( QWidget *parent, const QVariantList &args );
	~AppearanceConfig();

	virtual void save();
	virtual void load();

private slots:
	void slotHighlightChanged();
	void slotChangeFont();
	void slotEditTooltips();
	void emitChanged();
private:
	
private:
	class Private;
	Private * const d;
};
#endif
// vim: set noet ts=4 sts=4 sw=4:
