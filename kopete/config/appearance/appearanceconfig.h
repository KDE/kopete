/*
    appearanceconfig.h  -  Kopete Look Feel Config

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2005      by Michaël Larouche       <michael.larouche@kdemail.net>

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

#ifndef __APPEARANCE_H
#define __APPEARANCE_H

#include "kcmodule.h"

#include <qmap.h>


/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class AppearanceConfig : public KCModule
{
	Q_OBJECT

friend class KopeteStyleNewStuff;

public:
	AppearanceConfig( QWidget *parent, const char *name, const QStringList &args );
	~AppearanceConfig();

	virtual void save();
	virtual void load();

private slots:
	void slotSelectedEmoticonsThemeChanged();
	void slotTransparencyChanged(bool);
	void slotUpdatePreview();
	void slotHighlightChanged();
	void slotChangeFont();
	void slotDeleteStyle();
	void slotImportStyle();
	void slotCopyStyle();
	void slotStyleSelected();
	void slotVariantSelected(const QString &variantName);
	void slotEditTooltips();
	void emitChanged();
	void installNewTheme();
	void removeSelectedTheme();
	void slotGetThemes();
	void slotGetStyles();
	void slotLoadStyles();
	void updateEmoticonsButton(bool);
	
private:
	void updateEmoticonlist();
	void createPreviewChatSession();
	void createPreviewMessages();
	
private:
	class Private;
	Private *d;
};
#endif
// vim: set noet ts=4 sts=4 sw=4:
