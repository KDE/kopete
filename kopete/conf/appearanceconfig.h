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
#include <qptrlist.h>
#include <qmap.h>

class QFrame;
class QTabWidget;
class QCheckBox;
class QSplitter;
class KListBox;
class KIconView;
class KHTMLPart;
class KopeteContact;
class StyleEditDialog;
class QListBoxItem;

class AppearanceConfig_General;
class AppearanceConfig_ChatWindow;
class AppearanceConfig_ChatAppearance;
class AppearanceConfig_Contactlist;
class KopeteAwayConfigUI;

namespace KTextEditor {
	class View;
	class Document;
}

typedef QPtrList<KopeteContact> KopeteContactPtrList;
typedef QMap<QString,QString> KopeteChatStyleMap;

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

private slots:
	void slotConfigSound();
	void slotUseEmoticonsChanged(bool);
	void slotSelectedEmoticonsThemeChanged();
	void slotTransparencyChanged(bool);
	void slotUpdatePreview();
	void slotShowTrayChanged(bool);
	void slotHighlightChanged();
	void slotChangeFont();
	void slotAddStyle();
	void slotEditStyle();
	void slotDeleteStyle();
	void slotImportStyle();
	void slotCopyStyle();
	void slotStyleSaved();
	void slotStyleSelected();

private:
	QTabWidget* mAppearanceTabCtl; // The TabWidget

	// Widgets for Emoticon TAB
	QFrame* mEmoticonsTab;
	QCheckBox *mUseEmoticonsChk;
	KListBox *icon_theme_list;
	KIconView *icon_theme_preview;
// 	QSplitter *icon_theme_splitter;
	KHTMLPart *preview;
	KTextEditor::Document* editDocument;

	// All other TABs have their own ui-file
	AppearanceConfig_General *mPrfsGeneral;
	AppearanceConfig_Contactlist *mPrfsContactlist;
	AppearanceConfig_ChatAppearance *mPrfsChatAppearance;
	AppearanceConfig_ChatWindow *mPrfsChatWindow;
	KopeteAwayConfigUI *mAwayConfigUI;

	StyleEditDialog *styleEditor;
	QListBoxItem *editedItem;
	QMap<QListBoxItem*,QString> itemMap;
	QString currentStyle;

	void updateHighlight();
	bool errorAlert;
	QString fileContents( const QString &path );
	void addStyle( const QString &styleName, const QString &xslString );

};
#endif
// vim: set noet ts=4 sts=4 sw=4:
