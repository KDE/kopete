/*
    appearanceconfig.h  -  Kopete Look Feel Config

    Copyright (c) 2001-2002 by Duncan Mac-Vicar Prett <duncan@kde.org>
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

#ifndef __APPEARANCE_H
#define __APPEARANCE_H

#include "configmodule.h"
#include "qptrlist.h"

class QFrame;
class KTabCtl;
class QCheckBox;
class KListBox;
class KHTMLPart;
class KopeteContact;

class AppearanceConfig_General;
class AppearanceConfig_ChatWindow;
class AppearanceConfig_ChatAppearance;
class AppearanceConfig_Contactlist;
class KopeteAwayConfigUI;

typedef QPtrList<KopeteContact> KopeteContactPtrList;

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class AppearanceConfig : public ConfigModule
{
	Q_OBJECT

public:
	AppearanceConfig(QWidget * parent);
	~AppearanceConfig();

	virtual void save();
	virtual void reopen();

public slots:
	void slotSelectKind(int k);

private slots:
	void slotConfigSound();
	void slotUseEmoticonsChanged(bool);
	void slotConfigChanged();
	void slotTransparencyChanged(bool);
	void slotUpdatePreview();
	void slotShowTrayChanged();
	void slotHighlightChanged();

private:
	KTabCtl* mAppearanceTabCtl; // The TabWidget

	// Widgets for Emoticon TAB
	QFrame* mEmoticonsTab;
	QCheckBox *mUseEmoticonsChk;
	KListBox *icon_theme_list;
	KHTMLPart *preview;

	// All other TABs have their own ui-file
	AppearanceConfig_General *mPrfsGeneral;
	AppearanceConfig_Contactlist *mPrfsContactlist;
	AppearanceConfig_ChatAppearance *mPrfsChatAppearance;
	AppearanceConfig_ChatWindow *mPrfsChatWindow;
	KopeteAwayConfigUI *mAwayConfigUI;

};
#endif

// vim: set noet ts=4 sts=4 sw=4:
