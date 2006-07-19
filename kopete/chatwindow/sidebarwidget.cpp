/*
    chatmemberslistwidget.cpp - Chat Members List Widget

    Copyright (c) 2004      by Richard Smith         <kde@metafoo.co.uk>

    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <QLayout>
#include <QTabWidget>

#include "khtml_part.h"
#include "khtmlview.h"

#include "klocale.h"

#include "sidebarwidget.h"
#include "kopetechatsession.h"
#include "chatmemberslistwidget.h"
#include "kopetechatwindow.h"

SidebarWidget::SidebarWidget( KopeteChatWindow *parent )
	 : m_chatWindow(parent), QDockWidget( parent )
{
	// The sidebar contains a QTabWidget (main widget).
	m_tabWidget = new QTabWidget(this);
	setWidget(m_tabWidget);
	
	// Here we add the first default page.
	QWidget *pageInfoZone = new QWidget(this);
	QVBoxLayout *l = new QVBoxLayout(pageInfoZone);
	m_htmlInfoZone = new KHTMLPart(pageInfoZone);
	l->addWidget(m_htmlInfoZone->view());
	
	m_tabWidget->addTab(pageInfoZone, i18n("Informations"));

	// The contact list
	ChatMembersListWidget *m_membersList = new ChatMembersListWidget(this);
	m_tabWidget->addTab(m_membersList, i18n("Chat members list"));

	connect(m_chatWindow, SIGNAL(chatSessionChanged(Kopete::ChatSession *)), this, SLOT(setChatSession(Kopete::ChatSession *)));
	connect(m_chatWindow, SIGNAL(chatSessionChanged(Kopete::ChatSession *)), m_membersList, SLOT(setChatSession(Kopete::ChatSession *)));

}

void SidebarWidget::setChatSession( Kopete::ChatSession *session )
{
	m_session = session;
}

SidebarWidget::~SidebarWidget()
{
}


#include "sidebarwidget.moc"

// vim: set noet ts=4 sts=4 sw=4:

