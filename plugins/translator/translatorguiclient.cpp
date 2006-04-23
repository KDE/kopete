/*
    translatorguiclient.cpp

    Kopete Translator plugin

    Copyright (c) 2003-2004 by Olivier Goffart <ogoffart @ kde.org>

    Kopete    (c) 2003-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qvariant.h>
//Added by qt3to4:
#include <Q3PtrList>

#include <kdebug.h>
#include <kaction.h>
#include <klocale.h>

#include "kopetechatsession.h"
#include "kopeteview.h"
#include "kopetecontact.h"
#include "kopetemetacontact.h"
#include "kopetemessage.h"

#include "translatorplugin.h"
#include "translatorguiclient.h"
#include "translatorlanguages.h"

TranslatorGUIClient::TranslatorGUIClient( Kopete::ChatSession *parent )
: QObject( parent ), KXMLGUIClient( parent )
{
	setInstance( TranslatorPlugin::plugin()->instance() );
	connect( TranslatorPlugin::plugin(), SIGNAL( destroyed( QObject * ) ), this, SLOT( deleteLater() ) );

	m_manager = parent;

	KAction *translate = new KAction( KIcon("locale"), i18n( "Translate" ), actionCollection(), "translateCurrentMessage" );
	connect( translate, SIGNAL( triggered(bool) ), this, SLOT( slotTranslateChat() ) );
	translate->setShortcut( KShortcut(Qt::CTRL + Qt::Key_T) );
	
	setXMLFile( "translatorchatui.rc" );
}

TranslatorGUIClient::~TranslatorGUIClient()
{
}

void TranslatorGUIClient::slotTranslateChat()
{
	if ( !m_manager->view() )
		return;

	Kopete::Message msg = m_manager->view()->currentMessage();
	QString body = msg.plainBody();
	if ( body.isEmpty() )
		return;

	QString src_lang = TranslatorPlugin::plugin()->m_myLang;
	QString dst_lang;

	QList<Kopete::Contact*> list = m_manager->members();
	Kopete::MetaContact *to = list.first()->metaContact();
	dst_lang = to->pluginData( TranslatorPlugin::plugin(), "languageKey" );
	if ( dst_lang.isEmpty() || dst_lang == "null" )
	{
		kDebug( 14308 ) << k_funcinfo << "Cannot determine dst Metacontact language (" << to->displayName() << ")" << endl;
		return;
	}

	// We search for src_dst
	TranslatorPlugin::plugin()->translateMessage( body, src_lang, dst_lang, this, SLOT( messageTranslated( const QVariant & ) ) );
}

void TranslatorGUIClient::messageTranslated( const QVariant &result )
{
	QString translated = result.toString();
	if ( translated.isEmpty() )
	{
		kDebug( 14308 ) << k_funcinfo << "Empty string returned"  << endl;
		return;
	}

	//if the user close the window before the translation arrive, return
	if ( !m_manager->view() )
		return;

	Kopete::Message msg = m_manager->view()->currentMessage();
	msg.setBody( translated );
	m_manager->view()->setCurrentMessage( msg );
}

#include "translatorguiclient.moc"

// vim: set noet ts=4 sts=4 sw=4:

