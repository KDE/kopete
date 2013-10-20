/*
    chatwindowconfig.h  -  Kopete Look Feel Config

    Copyright (c) 2005-2006 by Michaël Larouche       <larouche@kde.org>
    Copyright (c) 2005-2006 by Olivier Goffart         <ogoffart at kde.org>
    Copyright (c) 2007      by Gustavo Pichorim Boiko  <gustavo.boiko@kdemail.net>

    Kopete    (c) 2005-2007 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef CHATWINDOWCONFIG_H
#define CHATWINDOWCONFIG_H

#include <QtCore/QPointer>

#include <kcmodule.h>
#include "ui_chatwindowconfig_style.h"
#include "ui_chatwindowconfig_emoticons.h"
#include "ui_chatwindowconfig_colors.h"
#include "ui_chatwindowconfig_tab.h"
#include <kopetechatwindowstyle.h>

class FakeProtocol;
class FakeAccount;
class FakeContact;
class ChatMessagePart;
class QTabWidget;

namespace Kopete { class MetaContact; class ChatSession; }

class ChatWindowConfig : public KCModule
{
	Q_OBJECT

friend class KopeteStyleNewStuff;

public:
	ChatWindowConfig( QWidget *parent, const QVariantList &args );
	~ChatWindowConfig();

	virtual void save();
	virtual void load();

private slots:
	//----- Style TAB ---------------------
	void slotInstallChatStyle();
	int installChatStyle(const KUrl &styleToInstall);
	void slotDeleteChatStyle();
	void slotChatStyleSelected(const QString &styleName);
	void slotChatStyleVariantSelected(const QString &variantName);
	void emitChanged();
	void slotGetChatStyles();
	void slotLoadChatStyles();
	void slotUpdateChatPreview();
	//----- Emoticons TAB ---------------------
	void slotManageEmoticonThemes();

private:
	//----- Style TAB ---------------------
	void createPreviewChatSession();
	void createPreviewMessages();
	//----- Emoticons TAB ---------------------
	void updateEmoticonList();
	
private:
	//----- TAB Widget ---------------------
	QTabWidget *m_tab;
	//----- Style TAB ----------------------
	Ui::ChatWindowConfig_Style m_styleUi;
	ChatMessagePart *m_preview;

	ChatWindowStyle::StyleVariants m_currentVariantMap;
	QPointer<ChatWindowStyle> m_currentStyle;
	bool m_loading;
	bool m_allowDownloadTheme;
	// For style preview
	FakeProtocol *m_previewProtocol;
	FakeAccount *m_previewAccount;
	Kopete::MetaContact *m_jackMetaContact;
	FakeContact *m_myself;
	FakeContact *m_jack;
	Kopete::ChatSession *m_previewChatSession;

	//----- Emoticons TAB ---------------------
	Ui::ChatWindowConfig_Emoticons m_emoticonsUi;

	//----- Colors TAB ------------------------
	Ui::ChatWindowConfig_Colors m_colorsUi;

	//----- Tab TAB ---------------------
	Ui::ChatWindowConfig_Tab m_tabUi;
};
#endif
