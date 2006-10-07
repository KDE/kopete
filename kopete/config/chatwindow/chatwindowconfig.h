/*
    appearanceconfig.h  -  Kopete Look Feel Config

    Copyright (c) 2005-2006 by Michaël Larouche       <larouche@kde.org>
    Copyright (c) 2005-2006 by Olivier Goffart         <ogoffart at kde.org>

    Kopete    (c) 2005-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef CHATWINCONF_H
#define CHATWINCONF_H

#include "kcmodule.h"
#include "ui_chatwindowconfig_style.h"
#include <kopetechatwindowstyle.h>

class FakeProtocol;
class FakeAccount;
class FakeContact;
class ChatMessagePart;

namespace Kopete { class MetaContact; class ChatSession; }

class ChatWindowConfig : public KCModule
{
	Q_OBJECT

friend class KopeteStyleNewStuff;

public:
	ChatWindowConfig( QWidget *parent, const QStringList &args );
	~ChatWindowConfig();

	virtual void save();
	virtual void load();

private slots:
	//----- Style TAB ---------------------
	void slotInstallChatStyle();
	void slotDeleteChatStyle();
	void slotChatStyleSelected();
	void slotChatStyleVariantSelected(const QString &variantName);
	void emitChanged();
	void slotGetChatStyles();
	void slotLoadChatStyles();
	void slotUpdateChatPreview();
private:
	void createPreviewChatSession();
	void createPreviewMessages();
	
private:
	//----- Style TAB ----------------------
	Ui::ChatWindowConfig_Style m_styleUi;
	ChatMessagePart *m_preview;
	
	// value is the style path
	QMap<Q3ListBoxItem*,QString> m_styleItemMap;
	ChatWindowStyle::StyleVariants m_currentVariantMap;
	ChatWindowStyle *m_currentStyle;
	bool m_loading;
	bool m_styleChanged;
	bool m_allowDownloadTheme;
	// For style preview
	FakeProtocol *m_previewProtocol;
	FakeAccount *m_previewAccount;
	Kopete::MetaContact *m_myselfMetaContact;
	Kopete::MetaContact *m_jackMetaContact;
	FakeContact *m_myself;
	FakeContact *m_jack;
	Kopete::ChatSession *m_previewChatSession;


};
#endif
