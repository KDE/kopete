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

#include "kcmodule.h"
#include <qptrlist.h>
#include <qmap.h>

class QFrame;
class QTabWidget;
class QCheckBox;
class KListBox;
class KTextEdit;
class KHTMLPart;
class StyleEditDialog;
class QListBoxItem;

class AppearanceConfig_Emoticons;
class AppearanceConfig_ChatWindow;
class AppearanceConfig_Colors;
class AppearanceConfig_ContactList;

class KopeteAppearanceConfigPrivate;

namespace KTextEditor
{
	class View;
	class Document;
}

typedef QMap<QString,QString> KopeteChatStyleMap;

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 */
class AppearanceConfig : public KCModule
{
	Q_OBJECT

public:
	AppearanceConfig( QWidget *parent, const char *name, const QStringList &args );
	~AppearanceConfig();

	virtual void save();
	virtual void load();

private slots:
	void slotUseEmoticonsChanged(bool);
	void slotSelectedEmoticonsThemeChanged();
	void slotTransparencyChanged(bool);
	void slotUpdatePreview();
	void slotHighlightChanged();
	void slotChangeFont();
	void slotAddStyle();
	void slotEditStyle();
	void slotDeleteStyle();
	void slotImportStyle();
	void slotCopyStyle();
	void slotStyleModified(const QString &);
	void slotStyleSelected();
	void slotEditTooltips();
	void emitChanged();
	void installNewTheme();
	void removeSelectedTheme();
	void slotGetThemes();

private:
	void updateHighlight();
	QString fileContents(const QString &path);
	bool addStyle(const QString &styleName, const QString &xslString);
	void updateEmoticonlist();

private:
	QTabWidget* mAppearanceTabCtl;

	// Widgets for Chat TAB
	KHTMLPart *preview;
	KTextEditor::Document* editDocument;

	// All other TABs have their own ui-file
	AppearanceConfig_Emoticons *mPrfsEmoticons;
	AppearanceConfig_ChatWindow *mPrfsChatWindow;
	AppearanceConfig_Colors *mPrfsColors;
	AppearanceConfig_ContactList *mPrfsContactList;

	// Vars used in ChatWindow TAB
	StyleEditDialog *styleEditor;
	QListBoxItem *editedItem;
	QMap<QListBoxItem*,QString> itemMap;
	QString currentStyle;
	bool loading;
	bool styleChanged;

	KopeteAppearanceConfigPrivate *d;
};
#endif
// vim: set noet ts=4 sts=4 sw=4:
