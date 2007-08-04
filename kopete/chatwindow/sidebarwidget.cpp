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
#include <QList>
#include <QTextDocument>
#include <QLabel>

#include "klocale.h"
#include "kurl.h"

#include "sidebarwidget.h"
#include "kopetechatsession.h"
#include "chatmemberslistwidget.h"
#include "kopetechatwindow.h"
#include "kopeteemoticons.h"
#include "kopetemetacontact.h"
#include "kopetepicture.h"



SidebarWidget::SidebarWidget( KopeteChatWindow *parent )
    : QDockWidget( parent )
    , m_chatWindow(parent)
{
	// The sidebar contains a QTabWidget (main widget).
	m_tabWidget = new QTabWidget(this);
	setWidget(m_tabWidget);
	
	// Here we add the first default page.
	QWidget *pageInfoZone = new QWidget(this);
	QVBoxLayout *l = new QVBoxLayout(pageInfoZone);
	//m_htmlInfoZone = new KHTMLPart(pageInfoZone);
	m_infoZone = new QLabel(pageInfoZone);
        m_infoZone->setWordWrap(true);
	l->addWidget(m_infoZone);
	
	m_tabWidget->addTab(pageInfoZone, i18n("Information"));

	// The contact list
	ChatMembersListWidget *m_membersList = new ChatMembersListWidget(this);
	m_tabWidget->addTab(m_membersList, i18n("Chat members list"));

	connect(m_chatWindow, SIGNAL(chatSessionChanged(Kopete::ChatSession *)), this, SLOT(setChatSession(Kopete::ChatSession *)));
	connect(m_chatWindow, SIGNAL(chatSessionChanged(Kopete::ChatSession *)), m_membersList, SLOT(setChatSession(Kopete::ChatSession *)));

	/* Be careful to not initialize here bad things. They generally go in
	   setChatSession
	*/
}
void SidebarWidget::generateContactDetails()
{
	Kopete::ContactPtrList members = m_session->members();
	Kopete::Contact *contact = members.first();
	Kopete::MetaContact* metaContact = contact->metaContact();

	QString content = QLatin1String("<html><head></head><body><div style=\"margin-left:10px;margin-right:10px;\"><br>");

	int w = 120;

	if ( ! metaContact->picture().image().isNull() )
        {
		QString photoName = QString(QLatin1String("kopete-metacontact-photo:%1")).arg( QLatin1String(QUrl::toPercentEncoding( metaContact->metaContactId()) ));
		content += QString(QLatin1String("<img src=\"%1\" style=\"margin-bottom:10px;\"><br>")).arg( photoName );
		 w = ( metaContact->picture().image().width() > 100 ) ? metaContact->picture().image().width() + 20 : 120;
        }

	QString displayName;
	Kopete::Emoticons *e = Kopete::Emoticons::self();
	QList<Kopete::Emoticons::Token> t = e->tokenize( metaContact->displayName());
	QList<Kopete::Emoticons::Token>::iterator it;
	for( it = t.begin(); it != t.end(); ++it )
	{
		if( (*it).type == Kopete::Emoticons::Image )
		{
			displayName += (*it).picHTMLCode;
		} else if( (*it).type == Kopete::Emoticons::Text )
		{
			displayName += Qt::escape( (*it).text );
		}
	}

	content += QString(QLatin1String("<b><font size=\"+1\">%1</font></b><br>")).arg( displayName );
	content += contact->toolTip() + QLatin1String("</div></body><html>");

	// adjust formatting for the rather narrow sidebar
	content = content.replace( "</b>", "</b><br>&nbsp;" );
	content = content.replace( "<nobr>", "" );
	content = content.replace( "</nobr>", "" );

	m_infoZone->setText( content );
	


}

void SidebarWidget::addPage( QWidget *widget, QString& name )
{
	m_tabWidget->addTab(widget, name);
}

void SidebarWidget::setChatSession( Kopete::ChatSession *session )
{
	m_session = session;
	generateContactDetails();
}

SidebarWidget::~SidebarWidget()
{
}


#include "sidebarwidget.moc"

// vim: set noet ts=4 sts=4 sw=4:

